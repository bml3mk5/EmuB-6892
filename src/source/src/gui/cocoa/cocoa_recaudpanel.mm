/** @file cocoa_recaudpanel.mm

 HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2016.11.05 -

 @brief [ record audio panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_recaudpanel.h"
#import "../../video/rec_audio.h"
#import "../../config.h"
#import "../../emu.h"
#import "../../clocale.h"

extern EMU *emu;

static const char *type_label[] = {
#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_AVKIT
	_T("avkit"),
#endif
#ifdef USE_REC_AUDIO_WAVE
	_T("wave"),
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	_T("ffmpeg"),
#endif
#endif
	NULL };
static const int type_ids[] = {
#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_AVKIT
	RECORD_AUDIO_TYPE_AVKIT,
#endif
#ifdef USE_REC_AUDIO_WAVE
	RECORD_AUDIO_TYPE_WAVE,
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	RECORD_AUDIO_TYPE_FFMPEG,
#endif
#endif
	0 };

@implementation CocoaRecAudioPanel
- (id)init
{
	int i;
	for(i=0; i < COCOA_RECAUDIO_LIBS; i++) {
		codnums[i] = 0;
		quanums[i] = 0;
		enables[i] = false;
		codbtn[i] = nil;
		quabtn[i] = nil;
	}

	int codnum = emu->get_parami(VM::ParamRecAudioCodec);
//	int quanum = emu->get_parami(VM::ParamRecAudioQuality);
	typnum = 0;
	for(i=0; type_ids[i] != 0; i++) {
		if (type_ids[i] == emu->get_parami(VM::ParamRecAudioType)) {
			typnum = i;
			codnums[i] = codnum;
//			quanums[i] = quanum;
			break;
		}
	}

	[super init];

	[self setTitleById:CMsg::Record_Sound];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *box_tab = [box_all addBox:TabViewBox :0 :COCOA_DEFAULT_MARGIN :_T("box_tab")];

	tabView = [CocoaTabView create];
	[box_tab addControl:tabView :300 :32];
	[view addSubview:tabView];
	CocoaRecAudioTabViewDelegate *dele = [[CocoaRecAudioTabViewDelegate alloc] initWithPanel:self];
	[tabView setDelegate:dele];


	NSTabViewItem *tab;
	CocoaView *tab_view;
	CocoaLabel *label;

	CocoaLayout *vbox;
	CocoaLayout *hbox;

	for(i=0; type_ids[i] != 0; i++) {
		tab = [tabView addTabItem:_tgettext(type_label[i])];
		tab_view = (CocoaView *)[tab view];

		enables[i] = emu->rec_sound_enabled(type_ids[i]);

		_TCHAR name[10];
		_stprintf(name, _T("V%d"), i);
		vbox = [box_tab addBox:VerticalBox :0 :0 :name];
		_stprintf(name, _T("H%d"), i);
		hbox = [vbox addBox:HorizontalBox :CenterPos | MiddlePos :0 :name];

		switch(type_ids[i]) {
		case RECORD_AUDIO_TYPE_WAVE:
			label = [CocoaLabel createI:CMsg::Select_a_sample_rate_on_sound_menu_in_advance];
			[hbox addControl:label];
			[tab_view addSubview:label];

			break;
		default:
			label = [CocoaLabel createI:CMsg::Codec_Type];
				[hbox addControl:label width:120];
			[tab_view addSubview:label];

			const char **codlbl = emu->get_rec_sound_codec_list(type_ids[i]);
			codbtn[i] = [CocoaPopUpButton createT:codlbl action:nil selidx:codnums[i]];
			[hbox addControl:codbtn[i] width:160];
			[tab_view addSubview:codbtn[i]];

//			label = [CocoaLabel create:re titleid:CMsg::Quality];
//			[tab_view addSubview:label];

//			const char **qualbl = emu->get_rec_audio_quality_list(type_ids[i]);
//			quabtn[i] = [CocoaPopUpButton create:re items:qualbl action:nil selidx:quanums[i]];
//			[tab_view addSubview:quabtn[i]];
			break;
		}

		if (!enables[i]) {
			hbox = [vbox addBox:HorizontalBox :0 :0 :_T("Hlib")];
			label = [CocoaLabel createI:CMsg::Need_install_library];
			[hbox addControl:label];
			[tab_view addSubview:label];
		}
	}

	// button

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
	CocoaButton *btnCancel = [CocoaButton createI:CMsg::Cancel action:@selector(dialogCancel:)];
	[hbox addControl:btnCancel :120 :32];
	[view addSubview:btnCancel];

	btnOK = [CocoaButton createI:CMsg::Start action:@selector(dialogOk:)];
	[hbox addControl:btnOK :120 :32];
	[view addSubview:btnOK];

	[box_all realize:self];

	return self;
}

- (NSInteger)runModal
{
	return type_ids[0] ? [NSApp runModalForWindow:self] : NSCancelButton;
}

- (void)close
{
	[NSApp stopModalWithCode:NSCancelButton];
	[super close];
}

- (void)dialogOk:(id)sender
{
	typnum = (int)[tabView indexOfTabViewItem:[tabView selectedTabViewItem]];
	int codnum = (int)[codbtn[typnum] indexOfSelectedItem];
//	int quanum = (int)[quabtn[typnum] indexOfSelectedItem];
	emu->set_parami(VM::ParamRecAudioType, type_ids[typnum]);
	emu->set_parami(VM::ParamRecAudioCodec, codnum);
//	emu->set_parami(VM::ParamRecAudioQuality, quanum);

    // OK button is pushed
	[NSApp stopModalWithCode:NSOKButton];
	[super close];
}

- (void)dialogCancel:(id)sender
{
    // Cancel button is pushed
	[self close];
}
- (void)selectTab:(int)num
{
	[btnOK setEnabled:enables[num]];
}

@end

@implementation CocoaRecAudioTabViewDelegate
- (id)initWithPanel:(CocoaRecAudioPanel *)new_panel
{
	[super init];
	panel = new_panel;
	return self;
}
- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
	[panel selectTab:(int)[tabView indexOfTabViewItem:tabViewItem]];
}

@end

