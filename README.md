# HITACHI BASIC MASTER LEVEL3 MARK5 Emulator

#### Copyright(C) Common Source Code Project, Sasaji 2011-2023 All Rights Reserved.

## ファイル構成

    docs/ .................. ドキュメント
    font/ .................. 画面フォント作成perlスクリプト
      kanji/ ............... 擬似漢字ROM作成プログラム
    source/
      data/ ................ データファイル
      include/ ............. インクルードファイル
      lib/ ................. ライブラリファイル
      locale/ .............. 翻訳用(gettext)
        list.xml ........... 言語一覧
        ja/
          bml3mk5_ja.qm .... Qt用 日本語翻訳ファイル(変換済み)
          bml3mk5_ja.ts .... Qt用 日本語翻訳ファイル
          LC_MESSAGES/
            bml3mk5.po ..... 日本語翻訳ファイル
            bml3mk5.mo ..... 日本語翻訳ファイル(変換済み)
      src/ ................. ソースファイル
        extra/ ............. その他ソース
        gui/ ............... GUI関連ソース
          agar/ ............ Agar関連ソース
          cocoa/ ........... Mac Cocoa関連ソース
          gtk_x11/ ......... Gtk+関連ソース
          qt/ .............. Qt GUI関連ソース
          windows/ ......... Windows GUI関連ソース
          wxwidgets/ ....... wxWidgets GUI関連ソース
        osd/ ............... OS依存関連ソース
          gtk/ ............. Gtk+依存関連ソース
          qt/ .............. Qt依存関連ソース
          SDL/ ............. SDL依存関連ソース
          windows/ ......... Windows依存関連ソース
          wxwidgets/ ....... wxWidgets依存関連ソース
        res/ ............... リソースファイル
        video/ ............. 録画用関連ソース
          avkit/ ........... AVkit(mac用)関連ソース
          cocoa/ ........... Cocoa(mac用)関連ソース
          ffmpeg/ .......... ffmpeg関連ソース
          mmf/ ............. media foundation (windows)関連ソース
          libpng/ .......... LibPNG関連ソース
          qt/ .............. Qt用関連ソース
          qtkit/ ........... qtkit(mac用)関連ソース
          vfw/ ............. video for windows関連ソース
          wave/ ............ waveフォーマット関連ソース
          windows/ ......... windows関連ソース
          wxwidgets/ ....... wxWidgets関連ソース
        vm/ ................ VMメインプログラムソース
      patch/ ............... パッチファイル
        agar-1.4.1.patch ... Agar-1.4.1用パッチファイル
        agar-r9049.patch ... Agar-1.4.2β(rev.9049)用パッチファイル
        SDL-1.2.15-mac-keyboard.patch ...
                             SDL macでfnキーを使えるようにするパッチファイル
        SDL_net-1.2.8.patch ... SDL_net-1.2.8用パッチファイル
        SDL2_net-2.0.0.patch .. SDL_net-2.0.0用パッチファイル
      Eclipse/ ............. Eclipseプロジェクトファイル
        sdl_linux/ ......... SDL linux用
        sdl_win/ ........... SDL Pleiades(Eclipse日本語版)用
        wx_linux/ .......... wxWidgets linux用
        wx_win/ ............ wxWidgets Pleiades(Eclipse日本語版)用   
      VC++2010/
        bml3mk5.vcxproj ...... Windows版 VC++2010用プロジェクトファイル
        bml3mk5_sdl.vcxproj .. SDL版 VC++2010用プロジェクトファイル
        bml3mk5_wx.vcxproj ... wxWidgets版 VC++2010用プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      VC++2013/
        bml3mk5.vcxproj ...... Windows版 VC++2013用プロジェクトファイル
        bml3mk5_sdl.vcxproj .. SDL版 VC++2013用プロジェクトファイル
        bml3mk5_wx.vcxproj ... wxWidgets版 VC++2013用プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      VC++2015/
        bml3mk5.vcxproj ...... Windows版 VC++2015用プロジェクトファイル
        bml3mk5_sdl.vcxproj .. SDL版 VC++2015用プロジェクトファイル
        bml3mk5_wx.vcxproj ... wxWidgets版 VC++2015用プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      VC++2017/
        bml3mk5.vcxproj ...... Windows版 VC++2017用プロジェクトファイル
        bml3mk5_sdl.vcxproj .. SDL版 VC++2017用プロジェクトファイル
        bml3mk5_wx.vcxproj ... wxWidgets版 VC++2017用プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      VC++2019/
        bml3mk5.vcxproj ...... Windows版 VC++2019用プロジェクトファイル
        bml3mk5_sdl.vcxproj .. SDL版 VC++2019用プロジェクトファイル
        bml3mk5_wx.vcxproj ... wxWidgets版 VC++2019用プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      Xcode/ ............... Xcode用プロジェクトファイル
      Makefile.xxx ......... 各OSごとのmakeファイル
      README.md ............ このファイル


## コンパイル方法

 * [Windows(VC++)版](src/source/README_WIN.md)

 * [SDL版](src/source/README_SDL.md)

 * [wxWidgets版](src/source/README_WX.md)

 * [Qt版](src/source/README_QT.md)


## 免責事項

* このソフトはフリーウェアです。ただし、著作権は放棄しておりません。
  実行モジュールについては作者Sasajiにあります。
  ソースコードについてはそれぞれの作者にあります。
* このソフトによって発生したいかなる損害についても著作権者は一切責任を負いません。
  このソフトを使用するにあたってはすべて自己責任で行ってください。
* 雑誌やネットなどに転載される場合、不特定多数の方に再配布を行う場合でも
  承諾の必要はありませんが、転載の旨をご連絡いただけたら幸いです。


==============================================================================

連絡先：
  Sasaji (sasaji@s-sasaji.ddo.jp)
  http://s-sasaji.ddo.jp/bml3mk5/
  (Twitter: http://twitter.com/bml3mk5)

==============================================================================

