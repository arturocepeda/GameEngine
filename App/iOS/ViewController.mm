
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  iOS
//
//  --- ViewController.mm ---
//
//////////////////////////////////////////////////////////////////


#import "ViewController.h"

#import "GERenderSystemES20.h"
#import "GEAudioSystem.h"
#import "GEInputSystem.h"
#import "GEStateManager.h"
#import "GETaskManager.h"
#import "GETimer.h"
#import "GETime.h"
#import "GEDevice.h"
#import "GEApplication.h"
#import "GELog.h"
#import "GEDistributionPlatform.h"

#import "config.h"


#define USE_BINARY_CONTENT 1


using namespace GE;
using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Audio;
using namespace GE::Input;


class iOSLogListener : public LogListener
{
public:
   virtual void onLog(LogType pType, const char* pMessage) override
   {
      if(pType == LogType::Info)
      {
         NSLog(@"[Info] %s", pMessage);
      }
      else if(pType == LogType::Warning)
      {
         NSLog(@"[Warning] %s", pMessage);
      }
      else if(pType == LogType::Error)
      {
         NSLog(@"[Error] %s", pMessage);
      }
   }
} gLogListener;


@interface ViewController () <UIAccelerometerDelegate>
{
   // Render system
   RenderSystemES20* cRender;
   
   // Audio system
   AudioSystem* cAudio;
   
   // Task management
   TaskManager* cTaskManager;
   
   // State management
   StateManager cStateManager;
   
   // Settings
   AppSettings gSettings;
   
   // Input management
   UITouch* iFingerID[GE_MAX_FINGERS];
   UIAccelerometer* uiAccel;
   
   Scaler* cPixelToScreenX;
   Scaler* cPixelToScreenY;
   
   // Timer
   Timer cTimer;
   double dTime;
   
   // Distribution platform
   DistributionPlatform cDistributionPlatform;
}

@property (strong, nonatomic) EAGLContext* context;
@property (strong, nonatomic) GLKBaseEffect* effect;

-(void) accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration;
-(Vector2) pixelToScreen:(const GE::Vector2&)pixelPosition;

@end


@implementation ViewController

@synthesize context = _context;
@synthesize effect = _effect;

-(BOOL) prefersStatusBarHidden
{
   return YES;
}

-(void) dealloc
{
   [_context release];
   [_effect release];
   [super dealloc];
}

-(void) viewDidLoad
{
   [super viewDidLoad];
   
   Application::Name = GE_APP_NAME;
   Application::ID = GE_APP_ID;
   Application::VersionString = GE_VERSION_STRING;
   Application::VersionNumber = GE_VERSION_NUMBER;
   
#if USE_BINARY_CONTENT
   Application::ContentType = ApplicationContentType::Bin;
#endif
   
   // set device screen size and orientation
   const CGSize& screenSize = [[UIScreen mainScreen] bounds].size;
#if defined (GE_ORIENTATION_PORTRAIT)
   Device::Orientation = DeviceOrientation::Portrait;
   const float screenWidth = std::min(screenSize.width, screenSize.height);
   const float screenHeight = std::max(screenSize.width, screenSize.height);
   const float aspectRatio = screenWidth / screenHeight;
#else
   Device::Orientation = DeviceOrientation::Landscape;
   const float screenWidth = std::max(screenSize.width, screenSize.height);
   const float screenHeight = std::min(screenSize.width, screenSize.height);
   const float aspectRatio = screenHeight / screenWidth;
#endif
   Device::ScreenWidth = (int)(screenWidth * [[UIScreen mainScreen] scale]);
   Device::ScreenHeight = (int)(screenHeight * [[UIScreen mainScreen] scale]);
   Device::AspectRatio = aspectRatio;
   
   // register the log listener
   Log::addListener(&gLogListener);
   
   // initialize the distribution platform
   cDistributionPlatform.init();
   
   // initialize the application
   Application::startUp(initAppModule);
   
   // initialize OpenGL
   self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
   
   if(!self.context)
      NSLog(@"Failed to create ES context");
   
   GLKView* view = (GLKView*)self.view;
   view.context = self.context;
   view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
   
   [EAGLContext setCurrentContext:self.context];
   
   // set frames per second
   self.preferredFramesPerSecond = gSettings.getInstance()->getTargetFPS();
   
   // enable multiple touch
   self.view.userInteractionEnabled = YES;
   self.view.multipleTouchEnabled = YES;
   self.view.exclusiveTouch = YES;
   
   // IDs for touch management
   for(int i = 0; i < GE_MAX_FINGERS; i++)
   {
      iFingerID[i] = 0;
   }
   
   // pixel space to screen space
   if(Device::Orientation == DeviceOrientation::Portrait)
   {
      cPixelToScreenX = new Scaler(0.0f, Device::getTouchPadWidth(), -1.0f, 1.0f);
      cPixelToScreenY = new Scaler(0.0f, Device::getTouchPadHeight(), Device::getAspectRatio(), -Device::getAspectRatio());
   }
   else
   {
      cPixelToScreenX = new Scaler(0.0f, Device::getTouchPadHeight(), -1.0f, 1.0f);
      cPixelToScreenY = new Scaler(0.0f, Device::getTouchPadWidth(), Device::getAspectRatio(), -Device::getAspectRatio());
   }
   
#ifdef GE_USE_ACCELEROMETER
   // accelerometer
   uiAccel = [UIAccelerometer sharedAccelerometer];
   uiAccel.updateInterval = GE_ACCELEROMETER_UPDATE;
   uiAccel.delegate = self;
#endif
   
   // initialize rendering system
   cRender = new RenderSystemES20();
   cRender->setBackgroundColor(Color(0.0f, 0.0f, 0.0f));
   
   // initialize audio system
   cAudio = new AudioSystem();
   cAudio->init();
   
   // start the timer
   cTimer.start();
   dTime = 0.0;
   Time::reset();
   
   // create the task manager
   cTaskManager = new TaskManager();
   
   // set the input device
   InputSystem::getInstance()->setCurrentInputDevice(InputDevice::Touchpad);
}

-(void) viewDidUnload
{
   [super viewDidUnload];
   
   [EAGLContext setCurrentContext:self.context];
   
   if([EAGLContext currentContext] == self.context)
      [EAGLContext setCurrentContext:nil];
   
   self.context = nil;
   
   delete cPixelToScreenX;
   delete cPixelToScreenY;

   Application::shutDown();
   
   cDistributionPlatform.shutdown();
}

-(void) didReceiveMemoryWarning
{
   [super didReceiveMemoryWarning];
}

-(BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
#ifdef GE_ORIENTATION_PORTRAIT
   if(interfaceOrientation == UIInterfaceOrientationPortrait)
      return YES;
   if(interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown)
      return YES;
#endif
#ifdef GE_ORIENTATION_LANDSCAPE
   if(interfaceOrientation == UIInterfaceOrientationLandscapeRight)
      return YES;
   if(interfaceOrientation == UIInterfaceOrientationLandscapeLeft)
      return YES;
#endif
   
   return NO;
}

-(void) accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration
{
   InputSystem::getInstance()->updateAccelerometerStatus(Vector3(acceleration.x, acceleration.y, acceleration.z));
}

-(void) update
{
   // delta time
   double dCurrentTime = cTimer.getTime();
   float fDeltaTime = (dCurrentTime - dTime) * 0.000001f;
   dTime = dCurrentTime;
   Time::setDelta(fDeltaTime);
   
   // state update
   cTaskManager->update();
}

-(void) glkView:(GLKView *)view drawInRect:(CGRect)rect
{
   cTaskManager->render();
   [self.context presentRenderbuffer:GL_RENDERBUFFER];
}

-(Vector2) pixelToScreen:(const GE::Vector2&)pixelPosition
{
   return Vector2((float)cPixelToScreenX->y(pixelPosition.X), (float)cPixelToScreenY->y(pixelPosition.Y));
}

-(void) touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
   NSArray* uiTouchesList = [touches allObjects];
   
   for(UITouch* uiTouch in uiTouchesList)
   {
      for(int i = 0; i < GE_MAX_FINGERS; i++)
      {
         if(iFingerID[i] == 0)
         {
            CGPoint cgPoint = [uiTouch locationInView: self.view];
            
            iFingerID[i] = uiTouch;
            const Vector2 screenPosition = [self pixelToScreen: Vector2(cgPoint.x, cgPoint.y)];
            InputSystem::getInstance()->inputTouchBegin(i, screenPosition);
            
            break;
         }
      }
   }
}

-(void) touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
   NSArray* uiTouchesList = [touches allObjects];
   
   for(UITouch* uiTouch in uiTouchesList)
   {
      for(int i = 0; i < GE_MAX_FINGERS; i++)
      {
         if(iFingerID[i] == uiTouch)
         {
            CGPoint cgPreviousPoint = [uiTouch previousLocationInView: self.view];
            CGPoint cgCurrentPoint = [uiTouch locationInView: self.view];
            
            const Vector2 screenPositionPrevious = [self pixelToScreen: Vector2(cgPreviousPoint.x, cgPreviousPoint.y)];
            const Vector2 screenPositionCurrent = [self pixelToScreen: Vector2(cgCurrentPoint.x, cgCurrentPoint.y)];
            
            InputSystem::getInstance()->inputTouchMove(i, screenPositionPrevious, screenPositionCurrent);
            
            break;
         }
      }
   }
}

-(void) touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
   NSArray* uiTouchesList = [touches allObjects];
   
   for(UITouch* uiTouch in uiTouchesList)
   {
      for(int i = 0; i < GE_MAX_FINGERS; i++)
      {
         if(iFingerID[i] == uiTouch)
         {
            CGPoint cgPoint = [uiTouch locationInView: self.view];
            
            iFingerID[i] = 0;
            const Vector2 screenPosition = [self pixelToScreen: Vector2(cgPoint.x, cgPoint.y)];
            InputSystem::getInstance()->inputTouchEnd(i, screenPosition);
            
            break;
         }
      }
   }
}

-(void) touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
   [self touchesEnded:touches withEvent:event];
}

@end
