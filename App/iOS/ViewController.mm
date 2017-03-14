
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
#import "GEAudioSystemOpenAL.h"
#import "GEInputSystem.h"
#import "GEStateManager.h"
#import "GETaskManager.h"
#import "GETimer.h"
#import "GETime.h"
#import "GEDevice.h"
#import "GEApplication.h"

#import "config.h"


using namespace GE;
using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Audio;
using namespace GE::Input;


@interface ViewController () <UIAccelerometerDelegate>
{
   // Render system
   RenderSystemES20* cRender;
   
   // Audio system
   AudioSystemOpenAL* cAudio;
   
   // Task management
   TaskManager* cTaskManager;
   
   // State management
   StateManager cStateManager;
   
   // Input management
   UITouch* iFingerID[GE_MAX_FINGERS];
   UIAccelerometer* uiAccel;
   
   Scaler* cPixelToScreenX;
   Scaler* cPixelToScreenY;
   
   // Timer
   Timer cTimer;
   double dTime;
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

#if defined (GE_BINARY_CONTENT)
    Application::ContentType = ApplicationContentType::Bin;
#endif

   Application::startUp();
   
   // initialize OpenGL
   self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
   
   if(!self.context)
      NSLog(@"Failed to create ES context");
   
   GLKView* view = (GLKView*)self.view;
   view.context = self.context;
   view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
   
   [EAGLContext setCurrentContext:self.context];
   
   // set frames per second
   self.preferredFramesPerSecond = GE_FPS;
   
   // enable multiple touch
   self.view.userInteractionEnabled = YES;
   self.view.multipleTouchEnabled = YES;
   self.view.exclusiveTouch = YES;
   
   // IDs for touch management
   for(int i = 0; i < GE_MAX_FINGERS; i++)
      iFingerID[i] = 0;
   
   // device orientation
#ifdef GE_ORIENTATION_PORTRAIT
   Device::Orientation = DeviceOrientation::Portrait;
#else
   Device::Orientation = DeviceOrientation::Landscape;
#endif

   // system language
   NSString* nsLanguage = [[NSLocale preferredLanguages] objectAtIndex:0];
   
   if([nsLanguage isEqualToString:@"en"])
      Device::Language = SystemLanguage::English;
   else if([nsLanguage isEqualToString:@"es"])
      Device::Language = SystemLanguage::Spanish;
   else if([nsLanguage isEqualToString:@"de"])
      Device::Language = SystemLanguage::German;
   
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
   cAudio = new AudioSystemOpenAL();
   cAudio->init();
   
   // create and register the states
   registerStates(cStateManager);
   
   // start the timer
   cTimer.start();
   dTime = 0.0;
   Time::reset();
   
   // create the task manager
   cTaskManager = new TaskManager();
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
   
   delete cRender;
   
   cAudio->release();
   delete cAudio;

   Application::shutDown();
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
            InputSystem::getInstance()->inputTouchBegin(i, [self pixelToScreen: Vector2(cgPoint.x, cgPoint.y)]);
            
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
            
            InputSystem::getInstance()->inputTouchMove(i,
                                                   [self pixelToScreen: Vector2(cgPreviousPoint.x, cgPreviousPoint.y)],
                                                   [self pixelToScreen: Vector2(cgCurrentPoint.x, cgCurrentPoint.y)]);
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
            InputSystem::getInstance()->inputTouchEnd(i, [self pixelToScreen: Vector2(cgPoint.x, cgPoint.y)]);
            
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
