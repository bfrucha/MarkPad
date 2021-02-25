//
//  main.m
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Conf.h"

#pragma GCC diagnostic ignored "-Wundeclared-selector"
#pragma clang diagnostic ignored "-Wundeclared-selector"

@interface MPApplication : NSApplication {}
@end

@implementation MPApplication

- (void) sendEvent:(NSEvent *)event {
  
  if ([event type] == NSEventTypeKeyDown) {
    
    if (([event modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask)
        == NSEventModifierFlagCommand) {
      
      NSString* ch = [event charactersIgnoringModifiers];
      if (ch.length == 1)
        switch ([ch characterAtIndex:0]) {
          case 'x':
            if ([self sendAction:@selector(cut:) to:nil from:self]) return;
            break;
          case 'c':
            if ([self sendAction:@selector(copy:) to:nil from:self]) return;
            break;
          case 'v':
            if ([self sendAction:@selector(paste:) to:nil from:self]) return;
            break;
          case 'z':
            if ([self sendAction:@selector(undo:) to:nil from:self]) return;
            break;
          case 'y':
            if ([self sendAction:@selector(redo:) to:nil from:self]) return;
            break;
          case 'a':
            if ([self sendAction:@selector(selectAll:) to:nil from:self]) return;
            break;
        }
    }
    else if (([event modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask)
             == (NSEventModifierFlagCommand | NSEventModifierFlagShift)) {
      
      if ([[event charactersIgnoringModifiers] isEqualToString:@"Z"]) {
        if ([self sendAction:@selector(redo:) to:nil from:self]) return;
      }
    }
  }
  
  [super sendEvent:event];
}

@end


int main(int argc, char *argv[])
{
  Conf::instance.init(argv[0]);
  return NSApplicationMain(argc, (const char **)argv);
}
