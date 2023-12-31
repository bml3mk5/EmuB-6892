/** @file cocoa_keybindpanel.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @brief [ keybind panel ]
 */

#ifndef COCOA_KEYBINDPANEL_H
#define COCOA_KEYBINDPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"
#import "../gui_keybinddata.h"
#import "../../vm/vm.h"

#define USE_NSCELL 1

typedef struct selected_st {
	int row;
	int col;
} selected_cell_t;

//@class CocoaTableFieldView;

/**
	@brief key table data for keybind dialog
*/
@interface CocoaTableData : NSObject <NSTableViewDataSource>
{
	KeybindData kbdata;

	selected_cell_t selected;
}
- (id)initWithValue:(int)new_tabnum;
- (Uint32)combi;
- (void)setCombi:(Uint32)value;
- (int)tab_num;
- (int)selectedCol;
- (void)setSelectCol:(int)col row:(int)row;
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView;
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)obj forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;

- (void)SetVmKeyMap:(Uint16 *)vmKeyMap :(int)size;
- (void)SetVmKey:(int)idx :(Uint16)code;
- (bool)SetVmKeyCode:(int)idx :(Uint16)code;
- (void)SetVkKeyMap:(Uint32 *)vkKeyMap;
- (void)SetVkKeyDefMap:(Uint32 *)vkKeyDefMap :(int)rows :(int)cols;
- (void)SetVkKeyPresetMap:(Uint32 *)vkKeyMap :(int)idx;
- (bool)SetVkKeyCode:(codecols_t *)obj :(Uint32)code :(char *)label;
- (bool)SetVkKeyCode:(int)row :(int)col :(Uint32)code :(char *)label;
- (bool)SetVkKeyCode:(Uint32)code :(char *)label;
- (bool)SetVkJoyCode:(Uint32)code :(char *)label;
- (void)loadDefaultPreset;
- (void)loadPreset:(int)idx;
- (void)savePreset:(int)idx;
- (void)SetData;

- (void)onClickCell:(NSTableView *)sender;
- (void)onDoubleClickCell:(NSTableView *)sender;

@end

#ifndef USE_NSCELL
/**
	@brief Text control in key table
*/
@interface CocoaNText : NSView
{
	CocoaTableData *dataSource;
	int col;
	int row;

	Uint32 *joy_stat;	// joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4-b31 = trigger #1-#28
	NSTimer *timer;

	NSAttributedString *str;

	BOOL selClick;
}
- (id)initWithDataSource:(NSTableView *)new_tbl data:(CocoaTableData *)new_data col:(NSTableColumn *)tableColumn row:(int)new_row;
- (void)dealloc;
- (BOOL)acceptsFirstResponder;
- (void)keyDown:(NSEvent *)event;
- (void)flagsChanged:(NSEvent *)event;
- (void)keyUp:(NSEvent *)event;
- (void)mouseDown:(NSEvent *)event;
- (void)updateJoy;
- (void)setString:(NSString *)aString;
- (void)drawRect:(NSRect)theRect;
//- (void)resetCursorRects;
@end
#else /* USE_NSCELL */
/**
	@brief Text control in key table
*/
@interface CocoaText : NSTextView
{
	CocoaTableData *dataSource;

	Uint32 *joy_stat;	// joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4-b31 = trigger #1-#28
	NSTimer *timer;
}
- (id)initWithDataSource:(CocoaTableData *)new_data;
- (void)dealloc;
- (void)keyDown:(NSEvent *)event;
- (void)flagsChanged:(NSEvent *)event;
- (void)keyUp:(NSEvent *)event;
- (BOOL)performKeyEquivalent:(NSEvent *)event;
- (void)mouseDown:(NSEvent *)event;
- (void)updateJoy;
@end

/**
	@brief text cell in key table
*/
@interface CocoaTextCell : NSTextFieldCell
- (id)init;
- (id)initWithCoder:(NSCoder *)decoder;
- (id)initTextCell:(NSString *)aString;
#if !defined(MAC_OS_X_VERSION_10_13)
- (id)initImageCell:(NSImage *)anImage;
#endif
- (void)selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength;
@end
#endif /* USE_NSCELL */

//@interface CocoaTableFieldView : NSTableView
//- (void)keyDown:(NSEvent *)event;
//- (BOOL)textShouldBeginEditing:(NSText *)textObject;
//@end

/**
	@brief Tables in keybind control
*/
@interface CocoaTableView : NSScrollView
+ (CocoaTableView *)create:(NSRect)re tabnum:(int)tab_num;
+ (CocoaTableView *)createW:(int)width height:(int)height tabnum:(int)tab_num;
- (CocoaTableData *)data;
- (void)deleteColumns;
@end

#ifndef USE_NSCELL
/**
	@brief Tables in keybind control
*/
@interface CocoaTableViewDelegate : NSObject<NSTableViewDelegate>
{
	NSMutableDictionary *cells;
}
- (id)init;
- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;
//- (void)tableView:(NSTableView *)tableView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *) tableColumn row:(NSInteger)row;
//- (void)tableViewSelectionIsChanging:(NSNotification *)notification;
- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row;
- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row;
@end
#endif

/**
	@brief Button control in keybind control
*/
@interface CocoaButtonAttr : NSObject
{
	int tabNum;
	int idx;
}
@property (nonatomic) int tabNum;
@property (nonatomic) int idx;
- (id)initWithValue:(int)new_tabnum :(int)new_idx;
@end

/**
	@brief Keybind dialog box
*/
@interface CocoaKeybindPanel : CocoaBasePanel
{
	CocoaTableView *tableView[KEYBIND_MAX_NUM];
	CocoaCheckBox *chkCombi[2];
}
- (id)init;
- (NSInteger)runModal;
- (void)close;
- (void)dialogCancel:(id)sender;
- (void)dialogOk:(id)sender;
- (void)loadDefaultPreset:(id)sender;
- (void)loadPreset:(id)sender;
- (void)savePreset:(id)sender;

@end

#endif /* COCOA_KEYBINDPANEL_H */
