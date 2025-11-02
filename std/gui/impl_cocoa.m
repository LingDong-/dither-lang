#import <Cocoa/Cocoa.h>

@protocol ControlRow <NSObject>
@property (readonly, copy) NSString *name;
- (id)value;
- (void)setValue:(id)value;
- (NSArray<NSView*>*)views;
@end

@interface SliderRow : NSObject <ControlRow>
@property (copy) NSString *name;
@property (strong) NSTextField *label;
@property (strong) NSSlider *slider;
@property (strong) NSTextField *valueField;
@property (strong) NSStepper *stepper;
@end

@interface IntSliderRow : NSObject <ControlRow>
@property (copy) NSString *name;
@property (strong) NSTextField *label;
@property (strong) NSSlider *slider;
@property (strong) NSTextField *valueField;
@property (strong) NSStepper *stepper;
@end

@interface CheckboxRow : NSObject <ControlRow>
@property (copy) NSString *name;
@property (strong) NSTextField *label;
@property (strong) NSButton *checkbox;
@end

@interface FieldRow : NSObject <ControlRow>
@property (copy) NSString *name;
@property (strong) NSTextField *label;
@property (strong) NSTextField *valueField;
@end


@implementation SliderRow
- (instancetype)initWithName:(NSString *)name min:(double)min max:(double)max value:(double)value target:(id)target action:(SEL)action {
  if ((self = [super init])) {
    _name = [name copy];
    CGFloat y = 0;
    _label = [[NSTextField alloc] initWithFrame:NSMakeRect(0, y, 80, 20)];
    [_label setStringValue:name];
    [_label setBezeled:NO];
    [_label setDrawsBackground:NO];
    [_label setEditable:NO];
    [_label setSelectable:NO];
    _label.lineBreakMode = NSLineBreakByTruncatingMiddle;
    _label.alignment = NSTextAlignmentRight;
    _slider = [NSSlider sliderWithValue:value minValue:min maxValue:max target:target action:action];
    [_slider setFrame:NSMakeRect(80, y, 110, 20)];
    [_slider setContinuous:YES];
    _valueField = [[NSTextField alloc] initWithFrame:NSMakeRect(190, y, 40, 24)];
    [_valueField setDoubleValue:value];
    [_valueField setTarget:target];
    [_valueField setAction:action];
    [_valueField setAlignment:NSTextAlignmentRight];
    _valueField.usesSingleLineMode = YES;
    // _valueField.lineBreakMode = NSLineBreakByTruncatingTail;
    _stepper = [[NSStepper alloc] initWithFrame:NSMakeRect(230, y, 20, 24)];
    [_stepper setMinValue:min];
    [_stepper setMaxValue:max];
    [_stepper setDoubleValue:value];
    [_stepper setIncrement:(max-min)/100.0];
    [_stepper setTarget:target];
    [_stepper setAction:action];
  }
  return self;
}
- (NSArray<NSView*>*)views {
  return @[self.label, self.slider, self.valueField, self.stepper];
}
- (id)value {
  return @(self.slider.doubleValue);
}
- (void)setValue:(id)value {
  double v = [value doubleValue];
  [self.slider setDoubleValue:v];
  [self.valueField setDoubleValue:v];
  [self.stepper setDoubleValue:v];
}
@end



@implementation IntSliderRow
- (instancetype)initWithName:(NSString *)name
                         min:(NSInteger)min
                         max:(NSInteger)max
                       value:(NSInteger)value
                      target:(id)target
                      action:(SEL)action
{
  if ((self = [super init])) {
    _name = [name copy];
    CGFloat y = 0;
    _label = [[NSTextField alloc] initWithFrame:NSMakeRect(0, y, 80, 20)];
    [_label setStringValue:name];
    [_label setBezeled:NO];
    [_label setDrawsBackground:NO];
    [_label setEditable:NO];
    [_label setSelectable:NO];
    _label.lineBreakMode = NSLineBreakByTruncatingMiddle;
    _label.alignment = NSTextAlignmentRight;
    _slider = [NSSlider sliderWithValue:value
                                minValue:min
                                maxValue:max
                                  target:target
                                  action:action];
    [_slider setFrame:NSMakeRect(80, y, 110, 20)];
    [_slider setContinuous:YES];
    [_slider setNumberOfTickMarks:(max - min + 1)];
    [_slider setAllowsTickMarkValuesOnly:YES];
    _valueField = [[NSTextField alloc] initWithFrame:NSMakeRect(190, y, 40, 24)];
    [_valueField setIntegerValue:value];
    [_valueField setTarget:target];
    [_valueField setAction:action];
    [_valueField setAlignment:NSTextAlignmentRight];
    _valueField.usesSingleLineMode = YES;
    _stepper = [[NSStepper alloc] initWithFrame:NSMakeRect(230, y, 20, 24)];
    [_stepper setMinValue:min];
    [_stepper setMaxValue:max];
    [_stepper setIntegerValue:value];
    [_stepper setIncrement:1];
    [_stepper setTarget:target];
    [_stepper setAction:action];
  }
  return self;
}

- (NSArray<NSView *> *)views {
  return @[self.label, self.slider, self.valueField, self.stepper];
}
- (id)value {
  return @((NSInteger)self.slider.integerValue);
}
- (void)setValue:(id)value {
  NSInteger v = [value integerValue];
  [self.slider setIntegerValue:v];
  [self.valueField setIntegerValue:v];
  [self.stepper setIntegerValue:v];
}
@end




@implementation CheckboxRow
- (instancetype)initWithName:(NSString *)name checked:(BOOL)checked target:(id)target action:(SEL)action {
  if ((self = [super init])) {
    _name = [name copy];

    _label = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 80, 20)];
    [_label setStringValue:name];
    [_label setBezeled:NO];
    [_label setDrawsBackground:NO];
    [_label setEditable:NO];
    [_label setSelectable:NO];
    _label.lineBreakMode = NSLineBreakByTruncatingMiddle;
    _label.alignment = NSTextAlignmentRight;

    _checkbox = [[NSButton alloc] initWithFrame:NSMakeRect(230, 0, 20, 20)];
    [_checkbox setButtonType:NSButtonTypeSwitch];
    [_checkbox setTitle:@""];
    [_checkbox setState:checked ? NSControlStateValueOn : NSControlStateValueOff];
    [_checkbox setTarget:target];
    [_checkbox setAction:action];
  }
  return self;
}
- (NSArray<NSView*>*)views {
  return @[self.label,self.checkbox];
}
- (id)value {
  return @([self.checkbox state] == NSControlStateValueOn);
}
- (void)setValue:(id)value {
  BOOL checked = [value boolValue];
  [self.checkbox setState:checked ? NSControlStateValueOn : NSControlStateValueOff];
}
@end



@implementation FieldRow
- (instancetype)initWithName:(NSString *)name
                       value:(NSString *)value
                      target:(id)target
                      action:(SEL)action
{
  if ((self = [super init])) {
    _name = [name copy];
    CGFloat y = 0;
    _label = [[NSTextField alloc] initWithFrame:NSMakeRect(0, y, 80, 20)];
    [_label setStringValue:name];
    [_label setBezeled:NO];
    [_label setDrawsBackground:NO];
    [_label setEditable:NO];
    [_label setSelectable:NO];
    _label.alignment = NSTextAlignmentRight;
    _valueField = [[NSTextField alloc] initWithFrame:NSMakeRect(80, y, 165, 24)];
    [_valueField setStringValue:(value ? value : @"")];
    [_valueField setTarget:target];
    [_valueField setAction:action];
    [_valueField setAlignment:NSTextAlignmentLeft];
    _valueField.usesSingleLineMode = YES;
  }
  return self;
}

- (NSArray<NSView *> *)views {
  return @[self.label, self.valueField];
}

- (id)value {
  return self.valueField.stringValue;
}

- (void)setValue:(id)value {
  if ([value isKindOfClass:[NSString class]]) {
    [self.valueField setStringValue:value];
  } else {
    [self.valueField setStringValue:[value description]]; // fallback
  }
}
@end




@interface DynamicPanelController : NSObject
@property (strong) NSPanel *panel;
@property (strong) NSMutableArray<id<ControlRow>> *rows;
@property (strong) NSMutableDictionary<NSString*, id<ControlRow>> *rowByName;
@property (assign) CGFloat nextY;
- (void)addRow:(id<ControlRow>)row;
- (id)valueForLabel:(NSString *)label;
- (void)setValue:(id)value forLabel:(NSString *)label;
@end

@implementation DynamicPanelController

- (instancetype)init {
  if ((self = [super init])) {
    _rows = [NSMutableArray array];
    _rowByName = [NSMutableDictionary dictionary];
    _nextY = 0;

    NSScreen *mainScreen = [NSScreen mainScreen];
    NSRect screenRect = [mainScreen frame];
    CGFloat x = 100;
    CGFloat y = NSMaxY(screenRect) - 100;

    NSRect frame = NSMakeRect(x,y, 250, 1);
    _panel = [[NSPanel alloc]
              initWithContentRect:frame
                        styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskHUDWindow | NSWindowStyleMaskUtilityWindow)
                          backing:NSBackingStoreBuffered
                            defer:NO];
    [_panel makeKeyAndOrderFront:nil];
    [_panel setTitle:@"Parameters"];
    [_panel setFloatingPanel:YES];
    [_panel setLevel:NSScreenSaverWindowLevel];
    [_panel setHidesOnDeactivate:NO];
    [_panel setCanHide:NO];
    [_panel setBecomesKeyOnlyIfNeeded:YES];
    [_panel setWorksWhenModal:YES];
    [_panel setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces];
  }
  return self;
}

- (void)addRow:(id<ControlRow>)row {
  CGFloat rowHeight = 25;
  NSView *cv = self.panel.contentView;

  NSRect pf = self.panel.frame;
  pf.size.height += rowHeight;
  pf.origin.y -= rowHeight;
  [self.panel setFrame:pf display:YES animate:NO];

  for (id<ControlRow> r in self.rows) {
    for (NSView *v in [r views]) {
      NSRect f = v.frame;
      f.origin.y = f.origin.y+rowHeight;
      [v setFrame:f];
      [cv addSubview:v];
    }
  }

  for (NSView *v in [row views]) {
    NSRect f = v.frame;
    f.origin.y = 0;
    [v setFrame:f];
    [cv addSubview:v];
  }

  [self.rows addObject:row];
  [self.rowByName setObject:row forKey:row.name];


}

- (void)controlChanged:(id)sender {
  for (id<ControlRow> row in self.rows) {
    if ([[row views] containsObject:sender]) {
      if ([row isKindOfClass:[SliderRow class]]) {
        SliderRow *sr = (SliderRow *)row;
        double v = sr.slider.doubleValue;
        if (sender == sr.valueField) v = sr.valueField.doubleValue;
        else if (sender == sr.stepper) v = sr.stepper.doubleValue;
        [sr setValue:@(v)];
      }else if ([row isKindOfClass:[IntSliderRow class]]) {
        IntSliderRow *sr = (IntSliderRow *)row;
        int v = sr.slider.intValue;
        if (sender == sr.valueField) v = sr.valueField.intValue;
        else if (sender == sr.stepper) v = sr.stepper.intValue;
        [sr setValue:@(v)];
      }
    }
  }
}
- (id)valueForLabel:(NSString *)label {
  id<ControlRow> row = self.rowByName[label];
  return [row value];
}
- (void)setValue:(id)value forLabel:(NSString *)label {
  id<ControlRow> row = self.rowByName[label];
  [row setValue:value];
}
@end








DynamicPanelController *panel;

void gui_impl_init(){
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  [NSApp finishLaunching];
  [NSApp activateIgnoringOtherApps:YES];

  panel = [[DynamicPanelController alloc] init];
}

void gui_impl__slider1f(char* name, float x, float l, float r){
  SliderRow *gain = [[SliderRow alloc] initWithName:[NSString stringWithUTF8String:name]
                                       min:l max:r value:x
                                       target:panel action:@selector(controlChanged:)];
  [panel addRow:gain];
}

void gui_impl__slider1i(char* name, int x, int l, int r){
  IntSliderRow *gain = [[IntSliderRow alloc] initWithName:[NSString stringWithUTF8String:name]
                                       min:l max:r value:x
                                       target:panel action:@selector(controlChanged:)];
  [panel addRow:gain];
}

void gui_impl__toggle1i(char* name, int b){
  CheckboxRow *gain = [[CheckboxRow alloc] initWithName:[NSString stringWithUTF8String:name]
                                       checked:b
                                       target:panel action:@selector(controlChanged:)];
  [panel addRow:gain];
}

void gui_impl__field1s(char* name, char* x){
  FieldRow *gain = [[FieldRow alloc] initWithName:[NSString stringWithUTF8String:name]
                                       value:[NSString stringWithUTF8String:x]
                                       target:panel action:@selector(controlChanged:)];
  [panel addRow:gain];
}


float gui_impl__get1f(char* name){
  return [[panel valueForLabel:[NSString stringWithUTF8String:name]] doubleValue];
}

int gui_impl__get1i(char* name){
  return [[panel valueForLabel:[NSString stringWithUTF8String:name]] intValue];
}

char* gui_impl__get1s(char* name){
  NSString* nss = [panel valueForLabel:[NSString stringWithUTF8String:name]];
  return (char*)([nss UTF8String]);
}

void gui_impl_poll(){
  @autoreleasepool {
    NSEvent *event;
    if ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
        untilDate:nil
        inMode:NSDefaultRunLoopMode
        dequeue:YES])) {
        [NSApp sendEvent:event];
    }
  }
}