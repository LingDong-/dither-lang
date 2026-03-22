#ifndef WK_WIDTH
#define WK_WIDTH 640
#endif

#ifndef WK_HEIGHT
#define WK_HEIGHT 480
#endif

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include <signal.h>

static NSString *const kBridgeJS = @""
"class EventEmitter {"
"  constructor() { this._listeners = {}; }"
"  on(event, fn) { (this._listeners[event] ||= []).push(fn); return this; }"
"  removeAllListeners(event) {"
"    if (event) delete this._listeners[event]; else this._listeners = {};"
"    return this;"
"  }"
"  emit(event, ...args) { for (const fn of (this._listeners[event] || [])) fn(...args); }"
"  destroy() { }"
"}"
"class ChildProcess extends EventEmitter {"
"  constructor(id) {"
"    super();"
"    this.pid = id;"
"    this.killed = false;"
"    this.exitCode = null;"
"    this.stdout = new EventEmitter();"
"    this.stderr = new EventEmitter();"
"    this.stdin = {"
"      write: (data) => {"
"          window.webkit.messageHandlers.native.postMessage({"
"          action: 'write', id: this.pid, data: String(data)"
"        });"
"      }"
"    };"
"  }"
"  kill(signal) {"
"    this.killed = true;"
"    window.webkit.messageHandlers.native.postMessage({"
"      action: 'kill', id: this.pid, signal: signal || 'SIGTERM'"
"    });"
"  }"
"}"
"const child_process = {"
"  _nextId: 1,"
"  _children: {},"
"  spawn(command, args, opts) {"
"    const id = this._nextId++;"
"    const cp = new ChildProcess(id);"
"    this._children[id] = cp;"
"    const msg = { action: 'spawn', id, command, args: args || [] };"
"    if (opts && opts.cwd) msg.cwd = String(opts.cwd);"
"    window.webkit.messageHandlers.native.postMessage(msg);"
"    return cp;"
"  }"
"};"
"const _pendingCallbacks = {};"
"let _cbId = 0;"
""
"function _nativeCall(action, params) {"
"  return new Promise((resolve, reject) => {"
"    const callbackId = String(++_cbId);"
"    _pendingCallbacks[callbackId] = { resolve, reject };"
"    window.webkit.messageHandlers.native.postMessage({"
"      action, callbackId, ...params"
"    });"
"  });"
"}"
"var fs = window.fs = {"
"  readFile(path) { return _nativeCall('readFile', { path }); },"
"  writeFile(path, data) { return _nativeCall('writeFile', { path, data: String(data) }); },"
"  appendFile(path, data) { return _nativeCall('appendFile', { path, data: String(data) }); },"
"  exists(path) { return _nativeCall('exists', { path }); },"
"  unlink(path) { return _nativeCall('unlink', { path }); },"
"};"
"var WK = window.WK = {"
"  getBundlePaths() { return _nativeCall('getBundlePaths'); },"
"  openFileDialog(opts = {}) { return _nativeCall('openFileDialog', opts);},"
"  saveFileDialog(opts = {}) { return _nativeCall('saveFileDialog', opts);},"
"  openFolderDialog(opts = {}) { return _nativeCall('openFolderDialog', opts);},"
"  setTitle(t) { window.webkit.messageHandlers.native.postMessage({ action: 'setTitle', title: String(t) }); },"
"};"
"window.__bridge = {"
"  onStdout(id, data) {"
"    const cp = child_process._children[id];"
"    if (cp) cp.stdout.emit('data', data);"
"  },"
"  onStderr(id, data) {"
"    const cp = child_process._children[id];"
"    if (cp) cp.stderr.emit('data', data);"
"  },"
"  onClose(id, code, signal) {"
"    const cp = child_process._children[id];"
"    if (cp) {"
"      cp.exitCode = code;"
"      cp.emit('close', code, signal);"
"      delete child_process._children[id];"
"    }"
"  },"
"  onError(id, errMsg) {"
"    const cp = child_process._children[id];"
"    if (cp) {"
"      cp.emit('error', new Error(errMsg));"
"      delete child_process._children[id];"
"    }"
"  },"
"  resolveCallback(callbackId, value) {"
"    const cb = _pendingCallbacks[callbackId];"
"    if (cb) { delete _pendingCallbacks[callbackId]; cb.resolve(value); }"
"  },"
"  rejectCallback(callbackId, errMsg) {"
"    const cb = _pendingCallbacks[callbackId];"
"    if (cb) { delete _pendingCallbacks[callbackId]; cb.reject(new Error(errMsg)); }"
"  }"
"};"
"console.log('[bridge] native bridge loaded');";


@interface ChildProcess : NSObject
@property (assign) NSInteger procId;
@property (strong) NSTask *task;
@property (strong) NSPipe *stdinPipe;
@property (strong) NSPipe *stdoutPipe;
@property (strong) NSPipe *stderrPipe;
@end
@implementation ChildProcess
@end

@interface DocWindow : NSObject
@property (strong) NSWindow *window;
@property (strong) WKWebView *webView;
@property (strong) NSMutableDictionary<NSNumber *, ChildProcess *> *children;
@end
@implementation DocWindow
- (instancetype)init {
  if (self = [super init]) { _children = [NSMutableDictionary new]; }
  return self;
}
@end

@interface AppDelegate : NSObject <NSApplicationDelegate,
                                    WKScriptMessageHandler,
                                    WKUIDelegate,
                                    NSWindowDelegate>
@property (strong) WKWebViewConfiguration *sharedConfig;
@property (strong) NSMutableArray<DocWindow *> *docWindows;
@property (strong) NSMutableArray<WKWebView *> *dyingWebViews;
@end

@implementation AppDelegate

- (DocWindow *)docWindowForWebView:(WKWebView *)webView {
  for (DocWindow *dw in self.docWindows) {
    if (dw.webView == webView) return dw;
  }
  return nil;
}

- (DocWindow *)createNewWindow {
  DocWindow *dw = [[DocWindow alloc] init];

  NSRect frame = NSMakeRect(0, 0, WK_WIDTH, WK_HEIGHT);
  dw.webView = [[WKWebView alloc] initWithFrame:frame configuration:self.sharedConfig];
  dw.webView.UIDelegate = self;

  NSString *resDir = [self resourceDir];
  NSString *htmlPath = [resDir stringByAppendingPathComponent:@"index.html"];
  if ([[NSFileManager defaultManager] fileExistsAtPath:htmlPath]) {
    [dw.webView loadFileURL:[NSURL fileURLWithPath:htmlPath]
    allowingReadAccessToURL:[NSURL fileURLWithPath:resDir]];
  } else {
    [dw.webView loadHTMLString:@"<h1>index.html not found</h1>"
                       baseURL:nil];
  }
  dw.window = [[NSWindow alloc]
      initWithContentRect:frame
      styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                  NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable)
      backing:NSBackingStoreBuffered defer:NO];
  dw.window.title = @"";
  dw.window.contentView = dw.webView;
  dw.window.delegate = self;
  dw.window.releasedWhenClosed = NO;
  if (self.docWindows.count > 0) {
      NSWindow *last = self.docWindows.lastObject.window;
      NSPoint origin = last.frame.origin;
      [dw.window setFrameOrigin:NSMakePoint(origin.x + 25, origin.y - 25)];
  } else {
      [dw.window center];
  }
  [self.docWindows addObject:dw];
  [dw.window makeKeyAndOrderFront:nil];
  NSLog(@"New window (total: %lu)", self.docWindows.count);
  return dw;
}

- (NSString *)macosDir {
  return [[[NSBundle mainBundle] bundlePath]
    stringByAppendingPathComponent:@"Contents/MacOS"];
}

- (NSString *)resourceDir {
  NSString *rp = [[NSBundle mainBundle] resourcePath];
  if (rp && [[NSFileManager defaultManager]
      fileExistsAtPath:[rp stringByAppendingPathComponent:@"index.html"]]) {
    return rp;
  }
  NSString *execPath = [[NSProcessInfo processInfo] arguments][0];
  execPath = [execPath stringByResolvingSymlinksInPath];
  if (![execPath isAbsolutePath]) {
    NSString *cwd = [[NSFileManager defaultManager] currentDirectoryPath];
    execPath = [cwd stringByAppendingPathComponent:execPath];
  }
  return [execPath stringByDeletingLastPathComponent];
}

- (NSString *)resolveCommand:(NSString *)command {
  NSFileManager *fm = [NSFileManager defaultManager];
  if ([command hasPrefix:@"/"]) return command;
  NSString *bundled = [[self macosDir] stringByAppendingPathComponent:command];
  if ([fm isExecutableFileAtPath:bundled]) {
    NSLog(@"Resolved '%@' = '%@' (bundled)", command, bundled);
    return bundled;
  }
  NSString *inRes = [[self resourceDir] stringByAppendingPathComponent:command];
  if ([fm isExecutableFileAtPath:inRes]) {
    NSLog(@"Resolved '%@' = '%@' (resources)", command, inRes);
    return inRes;
  }
  NSString *pathEnv = [NSProcessInfo processInfo].environment[@"PATH"] ?: @"/usr/bin:/usr/local/bin:/opt/homebrew/bin";
  for (NSString *dir in [pathEnv componentsSeparatedByString:@":"]) {
    NSString *full = [dir stringByAppendingPathComponent:command];
    if ([fm isExecutableFileAtPath:full]) {
      NSLog(@"Resolved '%@' = '%@' (PATH)", command, full);
      return full;
    }
  }
  NSLog(@"WARNING: Could not resolve '%@', using as-is", command);
  return command;
}

- (NSString *)resolveFilePath:(NSString *)path {
  if ([path hasPrefix:@"/"] || [path hasPrefix:@"~"]) {
    return [path stringByExpandingTildeInPath];
  }
  NSString *resolved = [[self resourceDir] stringByAppendingPathComponent:path];
  if (![[NSFileManager defaultManager] fileExistsAtPath:resolved]) {
    NSString *fromHome = [NSHomeDirectory() stringByAppendingPathComponent:path];
    if ([[NSFileManager defaultManager] fileExistsAtPath:fromHome]) {
      return fromHome;
    }
  }
  return resolved;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
  self.docWindows = [NSMutableArray new];
  self.dyingWebViews = [NSMutableArray new];
  self.sharedConfig = [[WKWebViewConfiguration alloc] init];
  [self.sharedConfig.userContentController addScriptMessageHandler:self name:@"native"];
  [self.sharedConfig.preferences setValue:@YES forKey:@"developerExtrasEnabled"];
  [self.sharedConfig.preferences setValue:@YES forKey:@"allowFileAccessFromFileURLs"];

  WKUserScript *bridgeScript = [[WKUserScript alloc]
    initWithSource:kBridgeJS
    injectionTime:WKUserScriptInjectionTimeAtDocumentStart
    forMainFrameOnly:YES];
  [self.sharedConfig.userContentController addUserScript:bridgeScript];

  DocWindow *dw = [self createNewWindow];
  [dw.window orderFrontRegardless];
  dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)),
    dispatch_get_main_queue(), ^{
      [NSApp activateIgnoringOtherApps:YES];
  });
}

- (void)windowWillClose:(NSNotification *)notification {
  NSWindow *closing = notification.object;
  DocWindow *dw = nil;
  for (DocWindow *d in self.docWindows) {
    if (d.window == closing) { dw = d; break; }
  }
  if (!dw) return;
  for (NSNumber *key in dw.children) {
    ChildProcess *cp = dw.children[key];
    if (cp.task.isRunning) [cp.task terminate];
  }
  [dw.children removeAllObjects];
  WKWebView *wv = dw.webView;
  [wv stopLoading];
  [wv loadHTMLString:@"" baseURL:nil];
  wv.UIDelegate = nil;
  closing.contentView = [[NSView alloc] initWithFrame:NSZeroRect];
  dw.webView = nil;
  [self.docWindows removeObject:dw];
  NSLog(@"Window closed (remaining: %lu)", self.docWindows.count);
  [self.dyingWebViews addObject:wv];
  dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(30.0 * NSEC_PER_SEC)),
    dispatch_get_main_queue(), ^{
      [self.dyingWebViews removeObject:wv];
      NSLog(@"WebView fully released");
    });
}


- (void)webView:(WKWebView *)webView
  runJavaScriptAlertPanelWithMessage:(NSString *)message
  initiatedByFrame:(WKFrameInfo *)frame
  completionHandler:(void (^)(void))completionHandler {
  NSAlert *alert = [[NSAlert alloc] init];
  alert.messageText = message;
  [alert addButtonWithTitle:@"OK"];
  [alert runModal];
  completionHandler();
}
- (void)webView:(WKWebView *)webView
  runJavaScriptConfirmPanelWithMessage:(NSString *)message
  initiatedByFrame:(WKFrameInfo *)frame
  completionHandler:(void (^)(BOOL))completionHandler {
  NSAlert *alert = [[NSAlert alloc] init];
  alert.messageText = message;
  [alert addButtonWithTitle:@"OK"];
  [alert addButtonWithTitle:@"Cancel"];
  completionHandler([alert runModal] == NSAlertFirstButtonReturn);
}
- (void)webView:(WKWebView *)webView
  runJavaScriptTextInputPanelWithPrompt:(NSString *)prompt
  defaultText:(NSString *)defaultText
  initiatedByFrame:(WKFrameInfo *)frame
  completionHandler:(void (^)(NSString *))completionHandler {
  NSAlert *alert = [[NSAlert alloc] init];
  alert.messageText = prompt;
  NSTextField *input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 260, 24)];
  input.stringValue = defaultText ?: @"";
  alert.accessoryView = input;
  [alert addButtonWithTitle:@"OK"];
  [alert addButtonWithTitle:@"Cancel"];
  completionHandler([alert runModal] == NSAlertFirstButtonReturn ? input.stringValue : nil);
}

- (void)userContentController:(WKUserContentController *)uc
    didReceiveScriptMessage:(WKScriptMessage *)message
{
  NSDictionary *msg = message.body;
  NSString *action = msg[@"action"];
  WKWebView *sender = message.webView;
  DocWindow *dw = [self docWindowForWebView:sender];

  if ([action isEqualToString:@"spawn"])  { [self handleSpawn:msg docWindow:dw webView:sender]; return; }
  if ([action isEqualToString:@"write"]) {
    ChildProcess *cp = dw.children[msg[@"id"]];
    if (cp) [cp.stdinPipe.fileHandleForWriting
      writeData:[msg[@"data"] dataUsingEncoding:NSUTF8StringEncoding]];
    return;
  }
  if ([action isEqualToString:@"kill"]) {
    ChildProcess *cp = dw.children[msg[@"id"]];
    if (!cp || !cp.task.isRunning) return;
    NSString *sig = msg[@"signal"];
    if ([sig isEqualToString:@"SIGKILL"])      kill(cp.task.processIdentifier, SIGKILL);
    else if ([sig isEqualToString:@"SIGINT"])   kill(cp.task.processIdentifier, SIGINT);
    else                                        [cp.task terminate];
    return;
  }

  NSString *cbId = msg[@"callbackId"];
  if ([action isEqualToString:@"readFile"]) {
    [self handleReadFile:[self resolveFilePath:msg[@"path"]] callbackId:cbId webView:sender]; return;
  }
  if ([action isEqualToString:@"writeFile"]) {
    [self handleWriteFile:[self resolveFilePath:msg[@"path"]] data:msg[@"data"] callbackId:cbId webView:sender]; return;
  }
  if ([action isEqualToString:@"appendFile"]) {
    [self handleAppendFile:[self resolveFilePath:msg[@"path"]] data:msg[@"data"] callbackId:cbId webView:sender]; return;
  }
  if ([action isEqualToString:@"exists"]) {
    NSString *path = [self resolveFilePath:msg[@"path"]];
    BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:path];
    [self resolveCallback:cbId value:(exists ? @"true" : @"false") webView:sender]; return;
  }
  if ([action isEqualToString:@"unlink"]) {
    NSString *path = [self resolveFilePath:msg[@"path"]];
    NSError *err = nil;
    [[NSFileManager defaultManager] removeItemAtPath:path error:&err];
    if (err) [self rejectCallback:cbId error:[err localizedDescription] webView:sender];
    else [self resolveCallback:cbId value:@"null" webView:sender];
    return;
  }
  if ([action isEqualToString:@"getBundlePaths"]) {
    NSDictionary *paths = @{
      @"resources": [self resourceDir],
      @"macos": [self macosDir],
      @"bundle": [[NSBundle mainBundle] bundlePath],
      @"home": NSHomeDirectory(),
      @"temp": NSTemporaryDirectory()
    };
    NSData *json = [NSJSONSerialization dataWithJSONObject:paths options:0 error:nil];
    NSString *jsonStr = [[NSString alloc] initWithData:json encoding:NSUTF8StringEncoding];
    [self resolveCallback:cbId value:jsonStr webView:sender];
    return;
  }
  if ([action isEqualToString:@"openFileDialog"]) {
    [self handleOpenFileDialog:msg callbackId:cbId webView:sender]; return;
  }
  if ([action isEqualToString:@"saveFileDialog"]) {
    [self handleSaveFileDialog:msg callbackId:cbId webView:sender]; return;
  }
  if ([action isEqualToString:@"openFolderDialog"]) {
    [self handleOpenFolderDialog:msg callbackId:cbId webView:sender]; return;
  }
  if ([action isEqualToString:@"setTitle"]) {
    NSString *title = msg[@"title"] ?: @"";
    dispatch_async(dispatch_get_main_queue(), ^{
      sender.window.title = title;
    });
    return;
  }
}

- (void)handleSpawn:(NSDictionary *)msg docWindow:(DocWindow *)dw webView:(WKWebView *)webView {
  NSNumber *procId = msg[@"id"];
  NSString *command = msg[@"command"];
  NSArray *args = msg[@"args"];

  NSString *resolvedCommand = [self resolveCommand:command];

  ChildProcess *cp = [[ChildProcess alloc] init];
  cp.procId = [procId integerValue];
  cp.task = [[NSTask alloc] init];
  cp.stdinPipe = [NSPipe pipe];
  cp.stdoutPipe = [NSPipe pipe];
  cp.stderrPipe = [NSPipe pipe];

  cp.task.executableURL = [NSURL fileURLWithPath:resolvedCommand];
  cp.task.arguments = args ?: @[];
  cp.task.standardInput = cp.stdinPipe;
  cp.task.standardOutput = cp.stdoutPipe;
  cp.task.standardError = cp.stderrPipe;

  NSString *cwd = msg[@"cwd"];
  if (cwd && cwd.length > 0) {
    cp.task.currentDirectoryURL = [NSURL fileURLWithPath:cwd];
  } else {
    cp.task.currentDirectoryURL = [NSURL fileURLWithPath:NSHomeDirectory()];
  }
  __weak typeof(self) weakSelf = self;
  __weak WKWebView *targetWebView = webView;
  __weak DocWindow *targetDW = dw;
  NSNumber *pid = procId;

  cp.stdoutPipe.fileHandleForReading.readabilityHandler = ^(NSFileHandle *fh) {
    NSData *data = [fh availableData];
    if (data.length == 0) { fh.readabilityHandler = nil; return; }
    [weakSelf callJS:@"__bridge.onStdout" procId:pid data:data webView:targetWebView];
  };
  cp.stderrPipe.fileHandleForReading.readabilityHandler = ^(NSFileHandle *fh) {
    NSData *data = [fh availableData];
    if (data.length == 0) { fh.readabilityHandler = nil; return; }
    [weakSelf callJS:@"__bridge.onStderr" procId:pid data:data webView:targetWebView];
  };
  cp.task.terminationHandler = ^(NSTask *task) {
    dispatch_async(dispatch_get_main_queue(), ^{
      NSString *signal = @"null";
      if (task.terminationReason == NSTaskTerminationReasonUncaughtSignal) {
        int status = task.terminationStatus;
        if (status == SIGKILL) signal = @"'SIGKILL'";
        else if (status == SIGTERM) signal = @"'SIGTERM'";
        else if (status == SIGINT) signal = @"'SIGINT'";
        else signal = [NSString stringWithFormat:@"'SIGNAL_%d'", status];
      }
      NSString *js = [NSString stringWithFormat:
        @"window.__bridge.onClose(%@, %d, %@)",
        pid, task.terminationStatus, signal];
      [targetWebView evaluateJavaScript:js completionHandler:nil];
      [targetDW.children removeObjectForKey:pid];
    });
  };

  NSError *err = nil;
  [cp.task launchAndReturnError:&err];
  if (err) {
    NSString *errStr = [self jsonEscape:[err localizedDescription]];
    NSString *js = [NSString stringWithFormat:@"window.__bridge.onError(%@, %@)", pid, errStr];
    dispatch_async(dispatch_get_main_queue(), ^{
      [webView evaluateJavaScript:js completionHandler:nil];
    });
    return;
  }
  dw.children[procId] = cp;
  NSLog(@"Spawned id=%@ pid=%d: %@ %@", procId, cp.task.processIdentifier, resolvedCommand, args);
}

- (void)handleReadFile:(NSString *)path callbackId:(NSString *)callbackId webView:(WKWebView *)webView {
  NSError *err = nil;
  NSString *content = [NSString stringWithContentsOfFile:path
                          encoding:NSUTF8StringEncoding error:&err];
  if (err) [self rejectCallback:callbackId error:[err localizedDescription] webView:webView];
  else [self resolveCallback:callbackId value:[self jsonEscape:content] webView:webView];
}

- (void)handleWriteFile:(NSString *)path data:(NSString *)data callbackId:(NSString *)callbackId webView:(WKWebView *)webView {
  NSError *err = nil;
  [data writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:&err];
  if (err) [self rejectCallback:callbackId error:[err localizedDescription] webView:webView];
  else [self resolveCallback:callbackId value:@"null" webView:webView];
}

- (void)handleAppendFile:(NSString *)path data:(NSString *)data callbackId:(NSString *)callbackId webView:(WKWebView *)webView {
  if (![[NSFileManager defaultManager] fileExistsAtPath:path]) {
    [data writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
  } else {
    NSFileHandle *fh = [NSFileHandle fileHandleForWritingAtPath:path];
    [fh seekToEndOfFile];
    [fh writeData:[data dataUsingEncoding:NSUTF8StringEncoding]];
    [fh closeFile];
  } 
  [self resolveCallback:callbackId value:@"null" webView:webView];
}

- (void)handleOpenFileDialog:(NSDictionary *)msg callbackId:(NSString *)callbackId webView:(WKWebView *)webView {
  NSOpenPanel *panel = [NSOpenPanel openPanel];
  panel.canChooseFiles = YES;
  panel.canChooseDirectories = NO;
  panel.allowsMultipleSelection = [msg[@"multiple"] boolValue];
  panel.message = msg[@"message"] ?: @"";
  panel.prompt = msg[@"buttonLabel"] ?: @"Open";
  NSArray *types = msg[@"types"];
  if (types && types.count > 0) {
    if (@available(macOS 11.0, *)) {
      NSMutableArray *utTypes = [NSMutableArray new];
      for (NSString *ext in types) {
        UTType *t = [UTType typeWithFilenameExtension:ext];
        if (t) [utTypes addObject:t];
      }
      if (utTypes.count > 0) panel.allowedContentTypes = utTypes;
    } else {
      panel.allowedFileTypes = types;
    }
  }
  NSString *startDir = msg[@"startDir"];
  if (startDir) {
    panel.directoryURL = [NSURL fileURLWithPath:[startDir stringByExpandingTildeInPath]];
  }

  NSWindow *parentWindow = webView.window;
  [panel beginSheetModalForWindow:parentWindow completionHandler:^(NSModalResponse result) {
    if (result == NSModalResponseOK) {
      if (panel.allowsMultipleSelection) {
        NSMutableArray *paths = [NSMutableArray new];
        for (NSURL *url in panel.URLs) {
          [paths addObject:url.path];
        }
        NSData *json = [NSJSONSerialization dataWithJSONObject:paths options:0 error:nil];
        NSString *jsonStr = [[NSString alloc] initWithData:json encoding:NSUTF8StringEncoding];
        [self resolveCallback:callbackId value:jsonStr webView:webView];
      } else {
        [self resolveCallback:callbackId value:[self jsonEscape:panel.URL.path] webView:webView];
      }
    } else {
      [self resolveCallback:callbackId value:@"null" webView:webView];
    }
  }];
}

- (void)handleSaveFileDialog:(NSDictionary *)msg callbackId:(NSString *)callbackId webView:(WKWebView *)webView {
  NSSavePanel *panel = [NSSavePanel savePanel];
  panel.canCreateDirectories = YES;
  panel.message = msg[@"message"] ?: @"";
  panel.prompt = msg[@"buttonLabel"] ?: @"Save";

  NSString *defaultName = msg[@"defaultName"];
  if (defaultName) panel.nameFieldStringValue = defaultName;

  NSString *startDir = msg[@"startDir"];
  if (startDir) {
    panel.directoryURL = [NSURL fileURLWithPath:[startDir stringByExpandingTildeInPath]];
  }

  NSArray *types = msg[@"types"];
  if (types && types.count > 0) {
    if (@available(macOS 11.0, *)) {
      NSMutableArray *utTypes = [NSMutableArray new];
      for (NSString *ext in types) {
        UTType *t = [UTType typeWithFilenameExtension:ext];
        if (t) [utTypes addObject:t];
      }
      if (utTypes.count > 0) panel.allowedContentTypes = utTypes;
    } else {
      panel.allowedFileTypes = types;
    }
  }

  NSWindow *parentWindow = webView.window;
  [panel beginSheetModalForWindow:parentWindow completionHandler:^(NSModalResponse result) {
    if (result == NSModalResponseOK) {
      [self resolveCallback:callbackId value:[self jsonEscape:panel.URL.path] webView:webView];
    } else {
      [self resolveCallback:callbackId value:@"null" webView:webView];
    }
  }];
}
- (void)handleOpenFolderDialog:(NSDictionary *)msg callbackId:(NSString *)callbackId webView:(WKWebView *)webView {
  NSOpenPanel *panel = [NSOpenPanel openPanel];
  panel.canChooseFiles = NO;
  panel.canChooseDirectories = YES;
  panel.allowsMultipleSelection = NO;
  panel.message = msg[@"message"] ?: @"";
  panel.prompt = msg[@"buttonLabel"] ?: @"Choose";

  NSString *startDir = msg[@"startDir"];
  if (startDir) {
    panel.directoryURL = [NSURL fileURLWithPath:[startDir stringByExpandingTildeInPath]];
  }

  NSWindow *parentWindow = webView.window;
  [panel beginSheetModalForWindow:parentWindow completionHandler:^(NSModalResponse result) {
    if (result == NSModalResponseOK) {
      [self resolveCallback:callbackId value:[self jsonEscape:panel.URL.path] webView:webView];
    } else {
      [self resolveCallback:callbackId value:@"null" webView:webView];
    }
  }];
}

- (void)fileNew:(id)sender {
  [self createNewWindow];
}

- (void)evalInKeyWindow:(NSString *)js {
  NSWindow *key = [NSApp keyWindow];
  for (DocWindow *dw in self.docWindows) {
    if (dw.window == key) {
      [dw.webView evaluateJavaScript:js completionHandler:nil];
      return;
    }
  }
}

- (void)fileOpen:(id)sender {
  [self evalInKeyWindow:@"window.WK && window.WK.onMenuEvent && window.WK.onMenuEvent('open')"];
}
- (void)fileSave:(id)sender {
  [self evalInKeyWindow:@"window.WK && window.WK.onMenuEvent && window.WK.onMenuEvent('save')"];
}
- (void)fileSaveAs:(id)sender {
  [self evalInKeyWindow:@"window.WK && window.WK.onMenuEvent && window.WK.onMenuEvent('saveAs')"];
}

- (void)resolveCallback:(NSString *)callbackId value:(NSString *)value webView:(WKWebView *)webView {
  NSString *js = [NSString stringWithFormat:
      @"window.__bridge.resolveCallback('%@', %@)", callbackId, value];
  dispatch_async(dispatch_get_main_queue(), ^{
      [webView evaluateJavaScript:js completionHandler:nil];
  });
}

- (void)rejectCallback:(NSString *)callbackId error:(NSString *)errorMsg webView:(WKWebView *)webView {
  NSString *escaped = [self jsonEscape:errorMsg];
  NSString *js = [NSString stringWithFormat:
      @"window.__bridge.rejectCallback('%@', %@)", callbackId, escaped];
  dispatch_async(dispatch_get_main_queue(), ^{
      [webView evaluateJavaScript:js completionHandler:nil];
  });
}

- (void)callJS:(NSString *)func procId:(NSNumber *)pid data:(NSData *)data webView:(WKWebView *)webView {
  NSString *str = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
  if (!str) str = [data base64EncodedStringWithOptions:0];
  NSString *escaped = [self jsonEscape:str];
  NSString *js = [NSString stringWithFormat:@"window.%@(%@, %@)", func, pid, escaped];
  dispatch_async(dispatch_get_main_queue(), ^{
      [webView evaluateJavaScript:js completionHandler:nil];
  });
}

- (NSString *)jsonEscape:(NSString *)str {
  NSData *jsonData = [NSJSONSerialization dataWithJSONObject:@[str] options:0 error:nil];
  NSString *jsonArray = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
  return [jsonArray substringWithRange:NSMakeRange(1, jsonArray.length - 2)];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app { return YES; }

- (void)applicationWillTerminate:(NSNotification *)notification {
  for (DocWindow *dw in self.docWindows) {
    for (NSNumber *key in dw.children) {
      ChildProcess *cp = dw.children[key];
      if (cp.task.isRunning) [cp.task terminate];
    }
  }
}

@end

static void setupMenuBar(void) {
  NSMenu *menuBar = [NSMenu new];

  NSMenuItem *appMenuItem = [NSMenuItem new];
  [menuBar addItem:appMenuItem];
  NSMenu *appMenu = [NSMenu new];
  [appMenu addItemWithTitle:@"About" action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
  [appMenu addItem:[NSMenuItem separatorItem]];
  [appMenu addItemWithTitle:@"Hide" action:@selector(hide:) keyEquivalent:@"h"];
  NSMenuItem *hideOthers = [appMenu addItemWithTitle:@"Hide Others"
    action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
  hideOthers.keyEquivalentModifierMask = NSEventModifierFlagCommand | NSEventModifierFlagOption;
  [appMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
  [appMenu addItem:[NSMenuItem separatorItem]];
  [appMenu addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
  appMenuItem.submenu = appMenu;

  NSMenuItem *fileMenuItem = [NSMenuItem new];
  [menuBar addItem:fileMenuItem];
  NSMenu *fileMenu = [[NSMenu alloc] initWithTitle:@"File"];
  [fileMenu addItemWithTitle:@"New"      action:@selector(fileNew:)   keyEquivalent:@"n"];
  [fileMenu addItemWithTitle:@"Open..."  action:@selector(fileOpen:)  keyEquivalent:@"o"];
  [fileMenu addItemWithTitle:@"Save"     action:@selector(fileSave:)  keyEquivalent:@"s"];
  // NSMenuItem *saveAs = [fileMenu addItemWithTitle:@"Save As..."
  //     action:@selector(fileSaveAs:) keyEquivalent:@"S"];
  // saveAs.keyEquivalentModifierMask = NSEventModifierFlagCommand | NSEventModifierFlagShift;
  fileMenuItem.submenu = fileMenu;

  NSMenuItem *editMenuItem = [NSMenuItem new];
  [menuBar addItem:editMenuItem];
  NSMenu *editMenu = [[NSMenu alloc] initWithTitle:@"Edit"];
  [editMenu addItemWithTitle:@"Undo"       action:@selector(undo:)      keyEquivalent:@"z"];
  [editMenu addItemWithTitle:@"Redo"       action:@selector(redo:)      keyEquivalent:@"Z"];
  [editMenu addItem:[NSMenuItem separatorItem]];
  [editMenu addItemWithTitle:@"Cut"        action:@selector(cut:)       keyEquivalent:@"x"];
  [editMenu addItemWithTitle:@"Copy"       action:@selector(copy:)      keyEquivalent:@"c"];
  [editMenu addItemWithTitle:@"Paste"      action:@selector(paste:)     keyEquivalent:@"v"];
  [editMenu addItemWithTitle:@"Delete"     action:@selector(delete:)    keyEquivalent:@""];
  [editMenu addItemWithTitle:@"Select All" action:@selector(selectAll:) keyEquivalent:@"a"];
  editMenuItem.submenu = editMenu;

  NSMenuItem *viewMenuItem = [NSMenuItem new];
  [menuBar addItem:viewMenuItem];
  NSMenu *viewMenu = [[NSMenu alloc] initWithTitle:@"View"];
  NSMenuItem *fs = [viewMenu addItemWithTitle:@"Toggle Full Screen"
    action:@selector(toggleFullScreen:) keyEquivalent:@"f"];
  fs.keyEquivalentModifierMask = NSEventModifierFlagCommand | NSEventModifierFlagControl;
  viewMenuItem.submenu = viewMenu;

  NSMenuItem *windowMenuItem = [NSMenuItem new];
  [menuBar addItem:windowMenuItem];
  NSMenu *windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
  [windowMenu addItemWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
  [windowMenu addItemWithTitle:@"Close"    action:@selector(performClose:)       keyEquivalent:@"w"];
  windowMenuItem.submenu = windowMenu;
  [NSApp setWindowsMenu:windowMenu];

  [NSApp setMainMenu:menuBar];
}

int main(int argc, const char *argv[]) {
  NSApplication *app = [NSApplication sharedApplication];
  [app setActivationPolicy:NSApplicationActivationPolicyRegular];
  setupMenuBar();
  AppDelegate *delegate = [[AppDelegate alloc] init];
  app.delegate = delegate;
  [app activateIgnoringOtherApps:YES];
  [app run];
  return 0;
}
