/** @file cocoa_seldrvpanel.mm

 HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.12.12 -

 @brief [ select drive panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_seldrvpanel.h"
#import "../../config.h"
#import "../../emu.h"
#import "../../clocale.h"

extern EMU *emu;

@implementation CocoaSelDrvPanel
@synthesize selectedDrive;
- (id)initWithPrefix:(int)defdrv prefix:(const char *)prefix
{
	[super init];

	selectedDrive = defdrv;

	[self setTitleById:CMsg::Select_Drive];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:HorizontalBox :0 :COCOA_DEFAULT_MARGIN];

	for(int drv=0; drv<MAX_DRIVE; drv++) {
		char label[64];
		if (prefix) {
			sprintf(label, "%s%d", prefix, drv);
		} else {
			sprintf(label, "%s%d", CMSG(Drive), drv);
		}
		CocoaButton *btn = [CocoaButton createT:label action:@selector(dialogOk:)];
		[btn setRelatedObject:(id)(intptr_t)drv];
		[box_all addControl:btn width:80];
//		if (selectedDrive == drv) {
//			[btn setHighlighted:YES];
//		}
		[view addSubview:btn];
	}

	[box_all realize:self];

	return self;
}

- (NSInteger)runModal
{
	return [NSApp runModalForWindow:self];
}

- (void)close
{
	[NSApp stopModalWithCode:NSOKButton];
	[super close];
}

- (void)dialogOk:(id)sender
{
	CocoaButton *btn = (CocoaButton *)sender;
	selectedDrive = (int)(intptr_t)[btn relatedObject];
	[self close];
}

@end
