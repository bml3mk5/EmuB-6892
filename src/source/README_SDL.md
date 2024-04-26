# HITACHI BASIC MASTER LEVEL3 MARK5 Emulator SDL edition

#### Copyright(C) Common Source Code Project, Sasaji 2012-2024 All Rights Reserved.

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
          LC_MESSAGES/
            bml3mk5.po ..... 日本語翻訳ファイル
            bml3mk5.mo ..... 日本語翻訳ファイル(変換済み)
      src/ ................. ソースファイル
        extra/ ............. その他ソース
        gui/ ............... GUI関連ソース
          agar/ ............ Agar関連ソース
          cocoa/ ........... Mac Cocoa関連ソース
          gtk_x11/ ......... Gtk+関連ソース
          windows/ ......... Windows GUI関連ソース
        osd/ ............... OS依存関連ソース
          gtk/ ............. Gtk+依存関連ソース
          SDL/ ............. SDL依存関連ソース
        res/ ............... リソースファイル
        video/ ............. 録画用関連ソース
          avkit/ ........... AVkit(mac用)関連ソース
          cocoa/ ........... Cocoa(mac用)関連ソース
          ffmpeg/ .......... ffmpeg関連ソース
          libpng/ .......... LibPNG関連ソース
          qtkit/ ........... qtkit(mac用)関連ソース
          vfw/ ............. video for windows関連ソース
          wave/ ............ waveフォーマット関連ソース
          windows/ ......... windows関連ソース
        vm/ ................ VMメインプログラムソース
      patch/ ............... パッチファイル
        agar-1.4.1.patch ... Agar-1.4.1用パッチファイル
        agar-r9049.patch ... Agar-1.4.2β(rev.9049)用パッチファイル
        SDL-1.2.15-mac-keyboard.patch ...
                             SDL macでfnキーを使えるようにするパッチファイル
        SDL_net-1.2.8.patch ... SDL_net-1.2.8用パッチファイル
        SDL-2.0.8-mac-keyboard.patch ...
                             SDL2 macでfnキーを使えるようにするパッチファイル
        SDL-2.26.5-mac-keyboard.patch ...
                             SDL2 macでfnキーを使えるようにするパッチファイル
        SDL2_net-2.0.0.patch .. SDL_net-2.0.0用パッチファイル
      Eclipse/ ............. Eclipseプロジェクトファイル
        sdl_linux/ ......... SDL linux用
        sdl_win/ ........... SDL Pleiades(Eclipse日本語版)用
      VC++2010/
        bml3mk5_sdl.vcxproj .. SDL版 VC++2010用プロジェクトファイル
      Xcode/ ............... Xcode用プロジェクトファイル
      Makefile.xxx ......... 各OSごとのmakeファイル
      README_SDL.md ........ このファイル


## コンパイル方法


----------------------------------------
### MacOSX版 ###

* 以下のバージョンがあります。

 + SDL2 + Cocoa版 -> Makefile.mac_cocoa2, Makefile.mac_cocoa2_dbgr

  * 上記以外のMakefileは現在メンテナンスしていません。

#### 1. 開発環境の構築

 * Xcode を インストールします。
   (Command Line Tools for Xcode は自動的にインストールされるようだ。)

 * コンパイルに必要なライブラリをインストールします。
   ターミナル上で行います。(Xcodeは使用しません。)

  + ソースコードからビルドする場合

   - SDL2-2.26.5

    1. パッチを適用します。

           patch -p 1 < SDL2-2.26.5-mac-keyboard.patch

    2. ビルド＆インストール

           ./configure
           make
           make install

   - SDL2_ttf-2.20.2

        ./configure
        make
        make install

   - FFmpeg-4.x (https://ffmpeg.org/)

    (FFmpegを使用しない場合、src/rec_video_defs.hにある #define USE_REC_VIDEO_FFMPEG を
    コメントアウトする。)

    * ヘッダファイルが必要です。
    includeフォルダにヘッダファイルを入れてください。

    * ビルド方法は以下を参考：
     [CompilationGuide/MacOSX - FFmpeg](http://trac.ffmpeg.org/wiki/CompilationGuide/MacOSX/)


  + Homebrewを使用する場合

   - sdl2, sdl2_ttf, ffmpeg@4 をインストールしてください。


#### 2. コンパイル（コマンドラインを使用する場合）

 ターミナル上で行います。

 * Homebrewからインストールした場合、Makefile中にある変数を変更してください。

  + SDLPATH を /opt/homebrew に変更

  + SDL_CFLAGS に -I/opt/homebrew/opt/ffmpeg@4/include を追加


 * sharedなバイナリを作成する場合

       make -f Makefile.xxx clean
       make -f Makefile.xxx install

  + installは、makeを実行したディレクトリの上にReleaseSHディレクトリを作成し、
  そこに必要なファイルをコピーします。
  (/usr/localにコピーはしません。)
  + インストール先を変更するには、Makefile中にある、SH_INSTALLDIRを変更
     してください。

 * staticなバイナリを作成する場合

       make -f Makefile.xxx st_clean
       make -f Makefile.xxx st_install

  + インストール先を変更するには、Makefile中にある、ST_INSTALLDIRを変更
    してください。



#### 3. コンパイル（Xcodeを使用する場合）

 * Xcodeフォルダにあるプロジェクトを開く。

 * ヘッダファイルやライブラリが/usr/local以外にあるときは
   BuildSettings -> Search Paths にある
   Header Search Paths と Library Search Paths を設定してください。


----------------------------------------
### Linux版 ###

* 以下のバージョンがあります。

 + GTK+3 + SDL2版 -> Makefile.linux_gtk_sdl2, Makefile.linux_gtk_sdl2_dbgr

  * 上記以外のMakefileは現在メンテナンスしていません。

#### 1. 開発環境の構築

 * ディストリビューションに付属する開発用ライブラリをインストール。

  + DebianやUbuntu: Synapticパッケージマネージャもしくはapt-getでインストール
  + RedhatやFedora: Yumなどでインストール

  + 必要なライブラリ
   - コンパイラ: gcc, g++, make
   - 画面系: gtk-3.0-dev, libsdl2-dev, libsdl2-ttf-dev
   - 録画用: libavcodec58-dev, libavutil56-dev, libavformat58-dev, libswscale5-dev
     (これらはffmpeg4のライブラリ)

#### 2. コンパイル

 ターミナル(端末)上で行います。

 * sharedなバイナリを作成する場合
  - ライブラリをパッケージからインストールしている場合はこちらでビルド。

         make -f Makefile.xxx clean
         make -f Makefile.xxx install

  * installは、makeを実行したディレクトリの上にReleaseディレクトリを作成し、
    そこに必要なファイルをコピーします。
  (/usr/localにコピーはしません。)
  * インストール先を変更するには、Makefile中にある、SH_INSTALLDIRを変更
    してください。

 * staticなバイナリを作成する場合

         make -f Makefile.xxx st_clean
         make -f Makefile.xxx st_install

  * インストール先を変更するには、Makefile中にある、ST_INSTALLDIRを変更
    してください。


----------------------------------------
### MSYS2 + MinGW64 (Windows)版 ###

* 以下のバージョンがあります。

 + SDL2 + Win GUI版 -> Makefile.win_gui2, Makefile.win_gui2_dbgr

  * 上記以外のMakefileは現在メンテナンスしていません。

#### 1. 開発環境の構築

 * MSYS2をインストール

  + インストーラに従ってインストールします。

 * MinGW64シェルを起動してコンパイルに必要なライブラリをインストールします。

  + pacmanを使用して以下をインストール
   - mingw-w64-x86_64-gcc
   - mingw-w64-x86_64-make
   - mingw-w64-x86_64-SDL2
   - mingw-w64-x86_64-SDL2_ttf
   - mingw-w64-x86_64-ffmpeg4.4

#### 2. コンパイル

 MinGW Shell上で行います。

 * 必要に応じてMakefile.xxxを変更します。

  * MinGWのバージョンが異なる場合、GCCLIBDIRを修正。
  * SH_LOCALDIR、ST_LOCALDIRのパスを修正。
    SDL, SDL_ttfなどをバイナリパッケージでインストールした場合、
    インストール先が/mingw64/lib/配下になるため

 * sharedなバイナリを作成する場合

       make -f Makefile.xxx clean
       make -f Makefile.xxx install

  * installは、makeを実行したディレクトリの上にReleaseSHディレクトリを作成し、
  そこに必要なファイルをコピーします。
  (/usr/localにコピーはしません。)
  * インストール先を変更するには、Makefile中にある、SH_INSTALLDIRを変更
     してください。

 * staticなバイナリを作成する場合

  * LOCALLIBSに必要なライブラリをすべて記述してください。

       make -f Makefile.xxx st_clean
       make -f Makefile.xxx st_install

  * installは、makeを実行したディレクトリの上にReleaseSTディレクトリを作成し、
  そこに必要なファイルをコピーします。
  (/usr/localにコピーはしません。)
  * インストール先を変更するには、Makefile中にある、ST_INSTALLDIRを変更
     してください。


----------------------------------------
### VC++(Windows)版 ###

#### 1. 開発環境の構築

* 必要なライブラリをVC++でビルドします。

 + SDL2-2.26.5

    ソースからインストール
      VisualCフォルダにあるSDL.slnを使用してビルド。
      出来たdll,libはlib/Release/x86にコピーしておく。

 + SDL2_ttf-2.20.2

    ソースからインストール
      VisualCフォルダにあるSDL_ttf.slnを使用してビルド。

 + FFmpeg-4.x (https://ffmpeg.org/)

   (FFmpegを使用しない場合、src/rec_video_defs.hにある #define USE_REC_VIDEO_FFMPEG を
    コメントアウトする。)

    * ヘッダファイルが必要です。
      includeフォルダにヘッダファイルを入れてください。
    * 64ビット版のsharedライブラリとincludeファイルは以下のサイトから入手できます。
      [CODEX FFMPEG](https://www.gyan.dev/ffmpeg/builds/)
      にある ffmpeg-4.4.1-full_build-shared.7z をダウンロードします。
    * ヘッダファイルがあれば、32ビット版のビルドもできます。
    * バージョン3のヘッダファイルでもビルドできます。
    * バージョン5のヘッダファイルでもビルドできますが動作するかは未確認。


#### 2. コンパイル

 * プロジェクトファイル(*_sdl.vcxproj)を使用してビルド。
  + 表示→プロパティマネージャを開き、Release下を開く。
  + ユーザーマクロに設定しているパスを変更する。


----------------------------------------
### FreeBSD版 ###

#### 1. 開発環境の構築

  Xorg, GNOMEなどをpackageやportsからインストールしておきます。

  インストールはlinux版を参考にして下さい。

#### 2. コンパイル

  GNU make (gmake) を使用してください。

  make方法はlinux版を参考にして下さい。



----------------------------------------
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

