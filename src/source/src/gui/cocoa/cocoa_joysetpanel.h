/** @file cocoa_joysetpanel.h

 SHARP X68000 Emulator 'eCZ-600'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2023.01.07 -

 @brief [ joypad setting panel ]
 */

#ifndef COCOA_JOYSETTINGPANEL_H
#define COCOA_JOYSETTINGPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"
#import "cocoa_keybindctrl.h"
#import "../../vm/vm_defs.h"
#import "../../emu.h"

/**
	@brief Joypad setting dialog box
*/
@interface CocoaJoySettingPanel : CocoaKeybindBasePanel
{
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2JOYSTICK)
	CocoaPopUpButton *pop[MAX_JOYSTICKS];
	CocoaSlider *mash[MAX_JOYSTICKS][KEYBIND_JOY_BUTTONS];
	CocoaSlider *axis[MAX_JOYSTICKS][6];
#endif
#ifdef USE_PIAJOYSTICK
#ifdef USE_JOYSTICKBIT
	CocoaCheckBox *chkPiaJoyNeg;
	CocoaRadioGroup *radPiaJoyConn;
#else
	CocoaCheckBox *chkPiaJoyNoIrq;
#endif
#endif
#ifdef USE_PSGJOYSTICK
#ifdef USE_JOYSTICKBIT
	CocoaCheckBox *chkPsgJoyNeg;
#endif
#endif
}
- (id)init;
- (void)dialogOk:(id)sender;
- (void)setData;
@end

#endif /* COCOA_JOYSETTINGPANEL_H */
