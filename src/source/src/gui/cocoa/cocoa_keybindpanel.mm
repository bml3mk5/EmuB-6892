/** @file cocoa_keybindpanel.mm

 HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @brief [ keybind panel ]
 */

#import <Carbon/Carbon.h>
#import "cocoa_gui.h"
#import "cocoa_keybindpanel.h"
#import "../../config.h"
#import "../../emu.h"
#import "../../clocale.h"
#import "cocoa_key_trans.h"

extern EMU *emu;

@implementation CocoaTableData
- (id)initWithValue:(int)new_tabnum
{
	[super init];

#ifdef USE_PIAJOYSTICKBIT
	kbdata.Init(emu, new_tabnum, new_tabnum == 0 ? 0 : 1, new_tabnum == 2 ? 2 : 0);
#else
	kbdata.Init(emu, new_tabnum, new_tabnum == 0 ? 0 : 1, new_tabnum == 2 ? 1 : 0);
#endif

	// get parameters
	selected.row = -1;
	selected.col = -1;

	return self;
}
- (int)tab_num
{
	return kbdata.tab_num;
}
- (int)selectedCol
{
	return selected.col;
}
- (void)setSelectCol:(int)col row:(int)row
{
	selected.col = col;
	selected.row = row;
}
- (Uint32)combi
{
	return kbdata.GetCombi();
}
- (void)setCombi:(Uint32)value
{
	kbdata.SetCombi(value);
}
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
	return kbdata.GetNumberOfRows();
}
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	char label[128];
	int col = 0;
	if ([[tableColumn identifier] isEqualToString:@"1"]) {
		col++;
	}
	if([[tableColumn identifier] isEqualToString:@"vmkey"]) {
		col=-1;
	}
	kbdata.GetCellString((int)row,col,label);
	return [NSString stringWithUTF8String:label];
}
- (void)tableView:(NSTableView *)tableView setObjectValue:(id)obj forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
}

- (void)SetVmKeyMap:(Uint16 *)vmKeyMap :(int)size
{
	kbdata.SetVmKeyMap(vmKeyMap,size);
}

- (void)SetVmKey:(int)idx :(Uint16)code
{
	kbdata.SetVmKey(idx,code);
}

- (bool)SetVmKeyCode:(int)idx :(Uint16)code
{
	return kbdata.SetVmKeyCode(idx,code);
}

- (void)SetVkKeyMap:(Uint32 *)vkKeyMap
{
	kbdata.SetVkKeyMap(vkKeyMap);
}

- (void)SetVkKeyDefMap:(Uint32 *)vkKeyDefMap :(int)rows :(int)cols
{
	kbdata.SetVkKeyDefMap(vkKeyDefMap,rows,cols);
}

- (void)SetVkKeyPresetMap:(Uint32 *)vkKeyMap :(int)idx
{
	kbdata.SetVkKeyPresetMap(vkKeyMap,idx);
}

- (bool)SetVkKeyCode:(codecols_t *)obj :(Uint32)code :(char *)label
{
	return kbdata.SetVkKeyCode(obj,code,label);
}

- (bool)SetVkKeyCode:(int)row :(int)col :(Uint32)code :(char *)label
{
	return kbdata.SetVkKeyCode(row,col,code,label);
}

- (bool)SetVkKeyCode:(Uint32)code :(char *)label
{
	if (selected.row < 0 || selected.col < 0) return false;
	return kbdata.SetVkKeyCode(selected.row,selected.col-1,code,label);
}

- (bool)SetVkJoyCode:(Uint32)code :(char *)label
{
	if (selected.row < 0 || selected.col < 0) return false;
	return kbdata.SetVkJoyCode(selected.row,selected.col-1,code,label);
}

- (void)loadDefaultPreset
{
	kbdata.LoadDefaultPreset();
}

- (void)loadPreset:(int)idx
{
	kbdata.LoadPreset(idx);
}

- (void)savePreset:(int)idx
{
	kbdata.SavePreset(idx);
}

- (void)SetData
{
	kbdata.SetData();
}

- (void)onClickCell:(NSTableView *)sender
{
#ifdef USE_NSCELL
	int row = (int)[sender clickedRow];
	int col = (int)[sender clickedColumn];

	selected.row = row;
	selected.col = col;

	if (col > 0) {
		[sender editColumn:col row:row withEvent:nil select:YES];
	}
#endif
}

- (void)onDoubleClickCell:(NSTableView *)sender
{
}
@end

#ifndef USE_NSCELL
@implementation CocoaNText
- (id)initWithDataSource:(NSTableView *)new_tbl data:(CocoaTableData *)new_data col:(NSTableColumn *)tableColumn row:(int)new_row
{
	[super init];
//	[self setEditable:YES];
//	[self setSelectable:YES];
	dataSource = new_data;

	int new_col = 0;
	if ([[tableColumn identifier] isEqualToString:@"1"]) {
		new_col++;
	}
	if([[tableColumn identifier] isEqualToString:@"vmkey"]) {
		new_col=-1;
	}
	col = new_col;
	row = new_row;

	if ([dataSource tab_num] != 0) {
		// update joystick status per 0.1 sec
		joy_stat = emu->joy_buffer();
		NSRunLoop *loop = [NSRunLoop currentRunLoop];
		timer = [NSTimer timerWithTimeInterval:0.1 target:self selector:@selector(updateJoy) userInfo:nil repeats:YES];
//		[loop addTimer:timer forMode:NSRunLoopCommonModes];
		[loop addTimer:timer forMode:NSModalPanelRunLoopMode];
	} else {
		joy_stat = NULL;
		timer = nil;
	}
//	[self setFocusRingType:NSFocusRingTypeDefault];
	[self setString:[dataSource tableView:new_tbl objectValueForTableColumn:tableColumn row:row]];

	selClick = FALSE;

	return self;
}
- (void)dealloc
{
	if (timer != nil) [timer invalidate];
	[super dealloc];
}
- (BOOL)acceptsFirstResponder
{
	return YES;
}
- (void)keyDown:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
		selClick = TRUE;
		[self setNeedsDisplay:YES];
	}
}
- (void)flagsChanged:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
		selClick = TRUE;
		[self setNeedsDisplay:YES];
	}
}
- (void)keyUp:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
		selClick = TRUE;
		[self setNeedsDisplay:YES];
	}
}
- (void)mouseDown:(NSEvent *)event
{
	int count = (int)[event clickCount];
	[dataSource setSelectCol:col+1 row:row];
	selClick = TRUE;
	[self setNeedsDisplay:YES];
	if (count != 2) return;
	char label[128];
	int tab = [dataSource tab_num];
	if (tab != 0) {
		[dataSource SetVkJoyCode:0:label];
	} else {
		[dataSource SetVkKeyCode:0:label];
	}
	[self setString:[NSString stringWithUTF8String:label]];
}
- (void)updateJoy
{
	if (joy_stat == NULL) return;

	char label[128];

	if (col >= 0) {
		emu->reset_joystick();
		if (joy_stat[col]) {
			[dataSource SetVkJoyCode:joy_stat[col]:label];
			[self setString:[NSString stringWithUTF8String:label]];
			[self setNeedsDisplay:YES];
		}
	}
}
- (void)setString:(NSString *)aString
{
	str = [[NSAttributedString alloc] initWithString:(aString ? aString : @"")];
}
- (void)drawRect:(NSRect)theRect
{
	NSRect re = [self bounds];

	[[NSColor whiteColor] set];
	NSRectFill(re);

	if (col >= 0) {
		if (selClick && self == [NSView focusView]) {
			NSSetFocusRingStyle(NSFocusRingBelow);
		}

		[[NSColor grayColor] set];
		NSFrameRect(re);
	}

	[str drawInRect: NSMakeRect(
		re.origin.x + 6.0, re.origin.y - 1.0,
		re.size.width, re.size.height)];

	[self setNeedsDisplay:NO];
}
//- (void)resetCursorRects
//{
//	NSRect rect = [self bounds];
//	NSCursor* cursor = [NSCursor IBeamCursor];
//	[self addCursorRect:rect cursor:cursor];
//}
@end
#else /* USE_NSCELL */
@implementation CocoaText
- (id)initWithDataSource:(CocoaTableData *)new_data
{
	[super init];
	[self setEditable:YES];
	[self setSelectable:YES];
//	[self setBordered:YES];
	dataSource = new_data;

	if ([dataSource tab_num] != 0) {
		// update joystick status per 0.1 sec
		joy_stat = emu->joy_buffer();
		NSRunLoop *loop = [NSRunLoop currentRunLoop];
		timer = [NSTimer timerWithTimeInterval:0.1 target:self selector:@selector(updateJoy) userInfo:nil repeats:YES];
//		[loop addTimer:timer forMode:NSRunLoopCommonModes];
		[loop addTimer:timer forMode:NSModalPanelRunLoopMode];
	} else {
		joy_stat = NULL;
		timer = nil;
	}

	return self;
}
- (void)dealloc
{
	if (timer != nil) [timer invalidate];
	[super dealloc];
}
- (void)keyDown:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
	}
}
- (void)flagsChanged:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
	}
}
- (void)keyUp:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
	}

}
- (BOOL)performKeyEquivalent:(NSEvent *)event
{
	return NO;
}
- (void)mouseDown:(NSEvent *)event
{
	int count = (int)[event clickCount];
	if (count != 2) return;
	char label[128];
	int tab = [dataSource tab_num];
	if (tab != 0) {
		[dataSource SetVkJoyCode:0:label];
	} else {
		[dataSource SetVkKeyCode:0:label];
	}
	[self setString:[NSString stringWithUTF8String:label]];
}
- (void)mouseMoved:(NSEvent *)event
{
	// keep allow cursor
	[[NSCursor arrowCursor] set];
}
- (void)cursorUpdate:(NSEvent *)event
{
	// keep allow cursor
	[[NSCursor arrowCursor] set];
}
- (void)updateJoy
{
	if (joy_stat == NULL) return;

	int col = [dataSource selectedCol];
	char label[128];

    if (col > 0) {
        emu->reset_joystick();
        if (joy_stat[col-1]) {
            [dataSource SetVkJoyCode:joy_stat[col-1]:label];
			[self setString:[NSString stringWithUTF8String:label]];
        }
    }
}

@end

@implementation CocoaTextCell
- (id)init
{
	[super init];
	[self setEditable:YES];
	[self setSelectable:YES];
	[self setBordered:YES];
	return self;
}
- (id)initWithCoder:(NSCoder *)decoder
{
	[super initWithCoder:decoder];
	[self setEditable:YES];
	[self setSelectable:YES];
	[self setBordered:YES];
	return self;
}
- (id)initTextCell:(NSString *)aString
{
//	[super init];
	[super initTextCell:aString];
	[self setEditable:YES];
	[self setSelectable:YES];
	[self setBordered:YES];
	return self;
}
#if !defined(MAC_OS_X_VERSION_10_13)
- (id)initImageCell:(NSImage *)anImage
{
//	[super init];
	[super initImageCell:anImage];
	[self setEditable:YES];
	[self setSelectable:YES];
	[self setBordered:YES];
	return self;
}
#endif
- (void)selectWithFrame:(NSRect)re inView:(NSView *)ctlView editor:(NSText *)text delegate:(id)obj start:(NSInteger)start length:(NSInteger)length
{
	NSTableView *view = (NSTableView *)ctlView;
	CocoaText *ntext = [[CocoaText alloc] initWithDataSource:[view dataSource]];
	[ntext setString:[text string]];
	[super selectWithFrame:re inView:ctlView editor:ntext delegate:obj start:start length:0];
}
@end
#endif /* USE_NSCELL */

//@implementation CocoaTableFieldView
//- (void)keyDown:(NSEvent *)event
//{
//	int i=0;
//}
//- (BOOL)textShouldBeginEditing:(NSText *)textObject
//{
//	return YES;
//}
//@end

@implementation CocoaTableView
+ (CocoaTableView *)create:(NSRect)re tabnum:(int)tab_num
{
	CocoaTableView *me = [CocoaTableView alloc];
	[me initWithFrame:re];
	[me createMain:re.size tabnum:tab_num];
	return me;
}
+ (CocoaTableView *)createW:(int)width height:(int)height tabnum:(int)tab_num
{
	CocoaTableView *me = [CocoaTableView alloc];
	[me init];
	NSSize sz = NSMakeSize(width, height);
	[me createMain:sz tabnum:tab_num];
	return me;
}
- (void)createMain:(NSSize)sz tabnum:(int)tab_num
{
	NSRect re_ctl;

	[self setBorderType:NSBezelBorder];

	re_ctl = NSMakeRect(0,0,sz.width,24);
	NSScroller *hs = [[NSScroller alloc] initWithFrame:re_ctl];
//	[self addSubview:hs];
	[self setHorizontalScroller:hs];
	[self setHasHorizontalScroller:YES];
	re_ctl = NSMakeRect(0,0,24,sz.height);
	NSScroller *vs = [[NSScroller alloc] initWithFrame:re_ctl];
//	[self addSubview:vs];
	[self setVerticalScroller:vs];
	[self setHasVerticalScroller:YES];

	[self setAutohidesScrollers:NO];
//	[self setAutoresizesSubviews:YES];

	re_ctl = NSMakeRect(10,10,sz.width,24);
	NSTableHeaderView *hv = [[NSTableHeaderView alloc] init];

	re_ctl = NSMakeRect(10,128,sz.width, sz.height);
	NSTableView *table = [[NSTableView alloc] init];
//	[me addSubview:table];
	[table setHeaderView:hv];

	[table setAllowsColumnReordering:NO];
	[table setAllowsColumnResizing:NO];
	[table setAllowsMultipleSelection:NO];
	[table setAllowsEmptySelection:YES];
//	[table setGridStyleMask:(NSTableViewSolidVerticalGridLineMask | NSTableViewSolidHorizontalGridLineMask)];
//	[table setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
	char label[128];
	CMsg::Id label_id;

	NSTableColumn *col;
	col = [[NSTableColumn alloc] initWithIdentifier:@"vmkey"];
#ifdef _MBS1
	if (tab_num == 2) label_id = CMsg::PIA_on_S1;
	else label_id = CMsg::S1_Key;
#else
	if (tab_num == 2) label_id = CMsg::PIA_on_L3;
	else label_id = CMsg::Level3_Key;
#endif
	[col.headerCell setStringValue:[NSString stringWithUTF8String:gMessages.Get(label_id)]];
	[col setWidth:120.0];
	[col setEditable:NO];
	[table addTableColumn:col];
	for(int i=0; i<2; i++) {
		sprintf(label, "%d", i);
		col = [[NSTableColumn alloc] initWithIdentifier:[NSString stringWithUTF8String:label]];
		if (tab_num != 0) sprintf(label, CMSG(JoypadVDIGIT), i+1);
		else sprintf(label, CMSG(BindVDIGIT), i+1);
		[col.headerCell setStringValue:[NSString stringWithUTF8String:label]];
#ifdef USE_NSCELL
		[col setDataCell:[[CocoaTextCell alloc] init]];
#endif
		[col setWidth:120.0];
		[col setEditable:YES];
		[table addTableColumn:col];
	}
	[self setDocumentView:table];
#ifndef USE_NSCELL
	[table setDelegate:[[CocoaTableViewDelegate alloc] init]];
#endif
	CocoaTableData *data = [[CocoaTableData alloc] initWithValue:tab_num];
	[table setDataSource:data];
	[table setTarget:data];
	[table setAction:@selector(onClickCell:)];
//	[table setDoubleAction:@selector(onDoubleClickCell:)];
}
- (CocoaTableData *)data
{
	return (CocoaTableData *)[[self documentView] dataSource];
}
- (void)deleteColumns
{
	NSTableView *table = [self documentView];
	NSArray *cols = [table tableColumns];
	NSUInteger count = [cols count];
	NSUInteger i;
	for  (i=0; i < count; i++) {
		[table removeTableColumn:[[table tableColumns] objectAtIndex:0]];
	}
}
@end

#ifndef USE_NSCELL
@implementation CocoaTableViewDelegate
- (id)init
{
	[super init];
	cells = [[NSMutableDictionary alloc] init];
	return self;
}
- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	CocoaTableData *data = [tableView dataSource];
	NSMutableString *idt = [NSMutableString stringWithString:[tableColumn identifier]];
	[idt appendFormat:@"-%d", (int)row];
	CocoaNText *ntext = [cells objectForKey:idt];
	if (ntext == nil) {
		ntext = [[CocoaNText alloc] initWithDataSource:tableView data:data col:tableColumn row:(int)row];
		[cells setObject:ntext forKey:idt];
	}
	return ntext;
}
//- (void)tableView:(NSTableView *)tableView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *) tableColumn row:(NSInteger)row
//{
//	NSLog(@"%@:%d",[tableColumn identifier], row);
//}
- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
	return NO;
}
- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	return 20.0;
}
@end
#endif

@implementation CocoaButtonAttr
@synthesize tabNum;
@synthesize idx;
- (id)initWithValue:(int)new_tabnum :(int)new_idx
{
	[super init];
	tabNum = new_tabnum;
	idx = new_idx;
	return self;
}
@end

@implementation CocoaKeybindPanel
- (id)init
{
	const CMsg::Id tab_labels[] = { CMsg::Keyboard,CMsg::Joypad_Key_Assigned,
#if (KEYBIND_MAX_NUM >= 3)
		CMsg::Joypad_PIA_Type,
#endif
		CMsg::End };

	[super init];

	SDL_QZ_InitOSKeymap();

	[self setTitleById:CMsg::Keybind];
	[self setShowsResizeIndicator:FALSE];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *box_tab = [box_all addBox:TabViewBox :0 :COCOA_DEFAULT_MARGIN :_T("box_tab")];
	CocoaLayout *box_sep;
	CocoaLayout *box_one;

	CocoaTabView *tabView = [CocoaTabView createI:tab_labels];
	[box_tab addControl:tabView :500 :400];
	[view addSubview:tabView];
	CocoaView *tab_view;

	CocoaButton *btn;
	char name[64];
	NSTabViewItem *tab;

	//
	for(int tab_num=0; tab_num<KEYBIND_MAX_NUM; tab_num++) {
		tab = [tabView tabViewItemAtIndex:tab_num];
		tab_view = (CocoaView *)[tab view];

		// table
		box_sep = [box_tab addBox:HorizontalBox :0 :0];
		box_one = [box_sep addBox:VerticalBox :0 :0];

		tableView[tab_num] = [CocoaTableView createW:390 height:400 tabnum:tab_num];
		[box_one addControl:tableView[tab_num] :390 :400];
		[tab_view addSubview:tableView[tab_num]];

		// checkbox

		if (tab_num == 2) {
			chkCombi[1] = [CocoaCheckBox createI:CMsg::Signals_are_negative_logic action:nil value:[[tableView[tab_num] data] combi]];
			[box_one addControl:chkCombi[1]];
			[tab_view addSubview:chkCombi[1]];
		} else if (tab_num == 1) {
			chkCombi[0] = [CocoaCheckBox createI:CMsg::Recognize_as_another_key_when_pressed_two_buttons action:nil value:[[tableView[tab_num] data] combi]];
			[box_one addControl:chkCombi[0]];
			[tab_view addSubview:chkCombi[0]];
		}

		// button (right side)

		box_one = [box_sep addBox:VerticalBox :0 :0];

		btn = [CocoaButton createI:CMsg::Load_Default action:@selector(loadDefaultPreset:)];
		[box_one addControl:btn width:180];
		[tab_view addSubview:btn];
		[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:tab_num:-1]];

		[box_one addSpace:180 :32];
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			sprintf(name, CMSG(Load_Preset_VDIGIT), i+1);
			btn = [CocoaButton createT:name action:@selector(loadPreset:)];
			[box_one addControl:btn width:180];
			[tab_view addSubview:btn];
			[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:tab_num:i]];

		}

		[box_one addSpace:180 :32];
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			sprintf(name, CMSG(Save_Preset_VDIGIT), i+1);
			btn = [CocoaButton createT:name action:@selector(savePreset:)];
			[box_one addControl:btn width:180];
			[tab_view addSubview:btn];
			[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:tab_num:i]];
		}
	}

	// button

	CocoaLayout *hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
	CocoaButton *btnCancel = [CocoaButton createI:CMsg::Cancel action:@selector(dialogCancel:)];
	[hbox addControl:btnCancel width:120];
	[view addSubview:btnCancel];

	CocoaButton *btnOK = [CocoaButton createI:CMsg::OK action:@selector(dialogOk:)];
	[hbox addControl:btnOK width:120];
	[view addSubview:btnOK];

	[box_all realize:self];

	return self;
}

- (NSInteger)runModal
{
	return [NSApp runModalForWindow:self];
}

- (void)close
{
	[NSApp stopModalWithCode:NSCancelButton];
	[super close];
}

- (void)dialogOk:(id)sender
{
    // OK button is pushed
	for(int tab=0; tab<KEYBIND_MAX_NUM; tab++) {
		CocoaTableData *data = [tableView[tab] data];
		[data SetData];
		if (tab == 1 || tab == 2) {
			[data setCombi:[chkCombi[tab-1] state] == NSOnState ? 1 : 0];
		}
	}

	emu->save_keybind();

	[NSApp stopModalWithCode:NSOKButton];
	[super close];
}

- (void)dialogCancel:(id)sender
{
    // Cancel button is pushed
	[self close];
}

- (void)loadDefaultPreset:(id)sender
{
	CocoaButtonAttr *attr = (CocoaButtonAttr *)[sender relatedObject];
	NSTableView *view = [tableView[attr.tabNum] documentView];
	CocoaTableData *data = [view dataSource];
	[view editColumn:0 row:[view selectedRow] withEvent:nil select:YES];
	[data loadDefaultPreset];
	if (attr.tabNum == 1 || attr.tabNum == 2) {
		[chkCombi[attr.tabNum-1] setState:[data combi]];
	}
	[view reloadData];
}

- (void)loadPreset:(id)sender
{
	CocoaButtonAttr *attr = (CocoaButtonAttr *)[sender relatedObject];
	NSTableView *view = [tableView[attr.tabNum] documentView];
	CocoaTableData *data = [view dataSource];
	[view editColumn:0 row:[view selectedRow] withEvent:nil select:YES];
	[data loadPreset:attr.idx];
	if (attr.tabNum == 1 || attr.tabNum == 2) {
		[chkCombi[attr.tabNum-1] setState:[data combi]];
	}
	[view reloadData];
}

- (void)savePreset:(id)sender
{
	CocoaButtonAttr *attr = (CocoaButtonAttr *)[sender relatedObject];
	NSTableView *view = [tableView[attr.tabNum] documentView];
	CocoaTableData *data = [view dataSource];
	[view editColumn:0 row:[view selectedRow] withEvent:nil select:YES];
	if (attr.tabNum == 1 || attr.tabNum == 2) {
		[data setCombi:[chkCombi[attr.tabNum-1] state] == NSOnState ? 1 : 0];
	}
	[data savePreset:attr.idx];
	[view reloadData];
}

@end
