/** @file cocoa_joysetpanel.mm

 SHARP X68000 Emulator 'eCZ-600'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2023.01.07 -

 @brief [ joypad setting panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_joysetpanel.h"
#import "../../config.h"
#import "../../emumsg.h"
#import "../../msgs.h"
#import "../../labels.h"
#import "../../keycode.h"
#import "../../utility.h"
#import "../gui_keybinddata.h"
#import "cocoa_key_trans.h"

extern EMU *emu;

@implementation CocoaJoySettingPanel
- (id)init
{
	char label[64];

	[super init];

	SDL_QZ_InitOSKeymap();

	[self setTitleById:CMsg::Joypad_Setting];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *box_hall = [box_all addBox:HorizontalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_hall")];
	CocoaLayout *hbox;

	int tx = 80;
	int sx = 140;
	int sy = 24;

#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		CocoaLayout *vbox = [box_hall addBox:VerticalBox];

		UTILITY::stprintf(label, 64, CMSG(JoypadVDIGIT), i + 1);
		[CocoaLabel createT:vbox title:label align:NSTextAlignmentCenter width:tx+sx height:sy];
		int val;
#ifdef USE_JOYSTICK_TYPE
		val = pConfig->joy_type[i];
		pop[i] = [CocoaPopUpButton createI:vbox items:LABELS::joypad_type action:nil selidx:val];
#else
		pop[i] = nil;
#endif

		hbox = [vbox addBox:HorizontalBox];
		[CocoaLabel createI:hbox title:CMsg::Button_Mashing_Speed];
		hbox = [vbox addBox:HorizontalBox];
		[hbox addSpace:tx :sy];
		[CocoaLabel createT:hbox title:_T("0 <-> 3")];

		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			int kk = k + VM_JOY_LABEL_BUTTON_A;
			if (kk >= VM_JOY_LABELS_MAX) {
				mash[i][k] = nil;
				continue;
			}

			hbox = [vbox addBox:HorizontalBox];
			CMsg::Id titleid = (CMsg::Id)cVmJoyLabels[kk].id;
			[CocoaLabel createI:hbox title:titleid align:NSTextAlignmentCenter width:tx height:sy];

			val = pConfig->joy_mashing[i][k];
			int n = i * KEYBIND_JOY_BUTTONS + k;
			CocoaSlider *slider = [CocoaSlider createN:hbox index:n action:nil min:0 max:3 value:val width:sx height:sy];
			[slider.cell setVertical:NO];
			[slider setAllowsTickMarkValuesOnly:YES];
			[slider setNumberOfTickMarks:4];
			mash[i][k] = slider;
		}

		hbox = [vbox addBox:HorizontalBox];
		[CocoaLabel createI:hbox title:CMsg::Analog_to_Digital_Sensitivity];
		hbox = [vbox addBox:HorizontalBox];
		[hbox addSpace:tx :sy];
		[CocoaLabel createT:hbox title:_T("0 <-> 10")];

		for(int k=0; k < 6; k++) {
			hbox = [vbox addBox:HorizontalBox];

			CMsg::Id titleid = LABELS::joypad_axis[k];
			[CocoaLabel createI:hbox title:titleid align:NSTextAlignmentCenter width:tx height:sy];

			val = 10 - pConfig->joy_axis_threshold[i][k];
			int n = i * 6 + k;
			CocoaSlider *slider = [CocoaSlider createN:hbox index:n action:nil min:0 max:10 value:val width:sx height:sy];
			[slider.cell setVertical:NO];
			[slider setAllowsTickMarkValuesOnly:YES];
			[slider setNumberOfTickMarks:11];
			axis[i][k] = slider;
		}
	}
#endif

	// tab control

	CocoaLayout *box_vall = [box_hall addBox:VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_vall")];
	CocoaLayout *box_tab = [box_vall addBox:TabViewBox :0 :COCOA_DEFAULT_MARGIN :_T("box_tab")];
	CocoaLayout *box_sep;
	CocoaLayout *box_one;

	tabView = [CocoaTabView create:box_tab width:370 height:260];
	CocoaView *view_in_tab;

	CocoaButton *btn;
	NSTabViewItem *tab;
	int tab_offset = KeybindData::JS_TABS_MIN;

	tableViews = [NSMutableArray array];
	for(int tab_num=tab_offset; tab_num<KeybindData::JS_TABS_MAX; tab_num++) {
		CocoaTableView *tableView = [CocoaTableView createW:370 height:260 tabnum:tab_num cellWidth:100];
		[tableView setJoyMask:&joy_mask];
		[tableViews addObject:tableView];
	}

	// create an item in the tab
	for(int tab_num=0; tab_num<[tableViews count]; tab_num++) {
//		tab = [tabView addTabItemI:LABELS::joysetting_tab[tab_num]];
		UTILITY::sprintf(label, sizeof(label), "%d", tab_num + 1);
		tab = [tabView addTabItemT:label];
		view_in_tab = (CocoaView *)[tab view];
		[box_tab setContentView:view_in_tab];

		box_sep = [box_tab addBox:VerticalBox :0 :0];

		// title
		[CocoaLabel createI:box_sep title:LABELS::joysetting_tab[tab_num]];

		// table
		CocoaTableView *tv = [tableViews objectAtIndex:tab_num];
		[box_sep addControl:tv width:370 height:260];

#if 0
		// checkbox
		if (![tv addCombiCheckButton:box_sep tabnum:tab_num+tab_offset]) {
			[CocoaLabel createT:box_sep title:""];
		}
#endif
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
		if (tab_num + KeybindData::JS_TABS_MIN == Keybind::TAB_JOY2JOY) {
# ifdef USE_JOYSTICKBIT
			// check box
			chkPiaJoyNeg = [CocoaCheckBox createI:box_sep title:CMsg::Signals_are_negative_logic action:nil value:FLG_PIAJOY_NEGATIVE != 0];
			CocoaLayout *hbox = [box_sep addBox:HorizontalBox :MiddlePos :0 :_T("ConnH")];
			[CocoaLabel createI:hbox title:CMsg::Connect_to_];
			radPiaJoyConn = [CocoaRadioGroup create:hbox width:360 cols:Config::PIAJOY_CONN_TO_MAX titleids:LABELS::joysetting_opts action:nil selidx:pConfig->piajoy_conn_to];
# else
			chkPiaJoyNoIrq = [CocoaCheckBox createI:box_sep title:CMsg::No_interrupt_caused_by_pressing_the_button action:nil value:FLG_PIAJOY_NOIRQ != 0];
# endif
		}
#endif
#if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
		if (tab_num + KeybindData::JS_TABS_MIN == Keybind::TAB_JOY2JOYB) {
# ifdef USE_JOYSTICKBIT
			// check box
			chkPsgJoyNeg = [CocoaCheckBox createI:box_sep title:CMsg::Signals_are_negative_logic action:nil value:FLG_PSGJOY_NEGATIVE != 0];
# endif
		}
#endif
	}


	// button (lower)

	btn = [CocoaButton createI:box_vall title:CMsg::Load_Default action:@selector(loadDefaultPreset:) width:160];
	[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:0:-1]];

	CocoaLayout *box_hbtn = [box_vall addBox:HorizontalBox :0 :0];
	box_one = [box_hbtn addBox:VerticalBox :0 :0];

	for(int i=0; i<KEYBIND_PRESETS; i++) {
		UTILITY::sprintf(label, sizeof(label), CMSG(Load_Preset_VDIGIT), i+1);
		btn = [CocoaButton createT:box_one title:label action:@selector(loadPreset:) width:160];
		[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:0:i]];

	}

	box_one = [box_hbtn addBox:VerticalBox :0 :0];

	for(int i=0; i<KEYBIND_PRESETS; i++) {
		UTILITY::sprintf(label, sizeof(label), CMSG(Save_Preset_VDIGIT), i+1);
		btn = [CocoaButton createT:box_one title:label action:@selector(savePreset:) width:160];
		[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:0:i]];
	}

	// axes of joypad
	[self createFooter:box_vall];

	// button

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
	[CocoaButton createI:hbox title:CMsg::Cancel action:@selector(dialogCancel:) width:120];
	[CocoaButton createI:hbox title:CMsg::OK action:@selector(dialogOk:) width:120];

	[box_all realize:self];

	return self;
}

- (void)dialogOk:(id)sender
{
	[self setData];
	[super dialogOk:sender];
}

- (void)setData
{
	[super setData];

#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
# ifdef USE_JOYSTICK_TYPE
		pConfig->joy_type[i] = (int)[pop[i] indexOfSelectedItem];
# endif
		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			pConfig->joy_mashing[i][k] = [mash[i][k] intValue];
		}
		for(int k=0; k<6; k++) {
			pConfig->joy_axis_threshold[i][k] = 10 - [axis[i][k] intValue];
		}
	}
	emu->modify_joy_mashing();
	emu->modify_joy_threshold();
# ifdef USE_JOYSTICK_TYPE
	// will change joypad type in emu thread
	emumsg.Send(EMUMSG_ID_MODIFY_JOYTYPE);
# endif
# if defined(USE_PIAJOYSTICK) || defined(USE_KEY2PIAJOYSTICK)
#  ifdef USE_JOYSTICKBIT
	BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NEGATIVE, [chkPiaJoyNeg state] != NSControlStateValueOff);
	pConfig->piajoy_conn_to = (int)[radPiaJoyConn selectedColumn];
#  else
	BIT_ONOFF(pConfig->misc_flags, MSK_PIAJOY_NOIRQ, [chkPiaJoyNoIrq state] != NSControlStateValueOff);
#  endif
# endif
# if defined(USE_PSGJOYSTICK) || defined(USE_KEY2PSGJOYSTICK)
#  ifdef USE_JOYSTICKBIT
	BIT_ONOFF(pConfig->misc_flags, MSK_PSGJOY_NEGATIVE, [chkPsgJoyNeg state] != NSControlStateValueOff);
#  endif
# endif
#endif
}

#if 0
- (void)changeSlider:(CocoaSlider *)sender
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	int i = [sender index];
	int k = i % KEYBIND_JOY_BUTTONS;
	i /= KEYBIND_JOY_BUTTONS;
	pConfig->joy_mashing[i][k] = [sender intValue];
	emu->set_joy_mashing();
#endif
}
#endif

@end
