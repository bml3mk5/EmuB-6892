/** @file cocoa_volumepanel.mm

 HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
 HITACHI MB-S1 Emulator 'EmuB-S1'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @brief [ volume panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_volumepanel.h"
#import "../../emu.h"
#import "../../labels.h"
#import "../../clocale.h"
#import "../../utility.h"

extern EMU *emu;

@implementation CocoaVolumePanel
- (id)init
{
	[super init];

	[self setPtr];

	[self setTitleById:CMsg::Volume];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *hbox;
	CocoaLayout *vbox;

	hbox = [box_all addBox:HorizontalBox];

	int n = 0;
	for(int i=0; LABELS::volume[i] != CMsg::End; i++) {
		bool wrap = (LABELS::volume[i] == CMsg::Null);
		if (wrap) {
			NSBox *sep = [[NSBox alloc] init];
			[box_all addControl:sep width:(i * 80) height:2];

			hbox = [box_all addBox:HorizontalBox];
			continue;
		}

		vbox = [hbox addBox:VerticalBox];
		[CocoaLabel createI:vbox title:LABELS::volume[i] align:NSTextAlignmentCenter width:80 height:32];

		CocoaSlider *slider = [CocoaSlider createN:vbox index:n action:@selector(changeSlider:) value:[self volume:n] width:80 height:120];
		if (NSAppKitVersionNumber > 1349.0 /* NSAppKitVersionNumber10_10_Max */) {
			[slider.cell setVertical:YES];
		}

		p_lbl[n] = [CocoaLabel createT:vbox title:"00" align:NSTextAlignmentCenter width:80 height:16];
		[self setVolumeText:n];

		[CocoaCheckBox createI:vbox title:CMsg::Mute index:n action:@selector(changeMute:) value:[self mute:n] width:80 height:32];

		if (i == 0) {
			// separator
			NSBox *sep = [[NSBox alloc] init];
			[hbox addControl:sep width:2 height:200];
		}
		n++;
	}

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
	[CocoaButton createI:hbox title:CMsg::Close action:@selector(dialogClose:) width:120];

	[box_all realize:self];

	return self;
}

- (NSInteger)runModal
{
	return [NSApp runModalForWindow:self];
}

- (void)close
{
	[NSApp stopModalWithCode:NSModalResponseOK];
	[super close];
}

- (void)dialogClose:(id)sender
{
	[self close];
}

- (void)setPtr
{
	int i = 0;
	p_volume[i++] = &pConfig->volume;
	p_volume[i++] = &pConfig->beep_volume;
#if defined(_MBS1)
	p_volume[i++] = &pConfig->psg_volume;
	p_volume[i++] = &pConfig->psgexfm_volume;
	p_volume[i++] = &pConfig->psgexssg_volume;
	p_volume[i++] = &pConfig->psgexpcm_volume;
	p_volume[i++] = &pConfig->psgexrhy_volume;
	p_volume[i++] = &pConfig->opnfm_volume;
	p_volume[i++] = &pConfig->opnssg_volume;
	p_volume[i++] = &pConfig->opnpcm_volume;
	p_volume[i++] = &pConfig->opnrhy_volume;
#endif
	p_volume[i++] = &pConfig->psg6_volume;
	p_volume[i++] = &pConfig->psg9_volume;
	p_volume[i++] = &pConfig->relay_volume;
	p_volume[i++] = &pConfig->cmt_volume;
	p_volume[i++] = &pConfig->fdd_volume;

	i = 0;
	p_mute[i++] = &pConfig->mute;
	p_mute[i++] = &pConfig->beep_mute;
#if defined(_MBS1)
	p_mute[i++] = &pConfig->psg_mute;
	p_mute[i++] = &pConfig->psgexfm_mute;
	p_mute[i++] = &pConfig->psgexssg_mute;
	p_mute[i++] = &pConfig->psgexpcm_mute;
	p_mute[i++] = &pConfig->psgexrhy_mute;
	p_mute[i++] = &pConfig->opnfm_mute;
	p_mute[i++] = &pConfig->opnssg_mute;
	p_mute[i++] = &pConfig->opnpcm_mute;
	p_mute[i++] = &pConfig->opnrhy_mute;
#endif
	p_mute[i++] = &pConfig->psg6_mute;
	p_mute[i++] = &pConfig->psg9_mute;
	p_mute[i++] = &pConfig->relay_mute;
	p_mute[i++] = &pConfig->cmt_mute;
	p_mute[i++] = &pConfig->fdd_mute;
}

- (void)setVolumeText:(int)idx
{
	char str[8];
	UTILITY::sprintf(str, sizeof(str), "%02d", [self volume:idx]);
	[p_lbl[idx] setStringValue:[NSString stringWithUTF8String:str]];
}

- (bool)mute:(int)idx
{
	return *p_mute[idx];
}

- (int)volume:(int)idx
{
	return *p_volume[idx];
}

- (void)changeSlider:(CocoaSlider *)sender
{
	*p_volume[sender.index] = [sender intValue];
	[self setVolumeText:sender.index];
	emu->set_volume(0);
}

- (void)changeMute:(CocoaCheckBox *)sender
{
	*p_mute[sender.index] = ([sender state] == NSControlStateValueOn);
	emu->set_volume(0);
}

@end
