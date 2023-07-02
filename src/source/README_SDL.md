# HITACHI BASIC MASTER LEVEL3 MARK5 Emulator SDL edition

#### Copyright(C) Common Source Code Project, Sasaji 2012-2023 All Rights Reserved.

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

 + SDL1 + Cocoa版 -> Makefile.mac_cocoa, Makefile.mac_cocoa_dbgr
 + SDL2 + Cocoa版 -> Makefile.mac_cocoa2, Makefile.mac_cocoa2_dbgr
 + SDL1 + Agar版  -> Makefile.mac_agar, Makefile.mac_agar_dbgr


#### 1. 開発環境の構築

 * Xcode を インストールします。
   (Command Line Tools for Xcode は自動的にインストールされるようだ。)

 * コンパイルに必要なライブラリをインストールします。
   ターミナル上で行います。(Xcodeは使用しません。)

  + SDL1の場合

   - SDL-1.2.15

    ソースからインストール
    1. パッチを適用します。

           patch -p 1 < SDL-1.2.15-mac-keyboard.patch

    2. ビルド＆インストール

           ./configure
           make
           make install

   - SDL_ttf-1.0.20

    ソースからインストール

        ./configure
        make
        make install

   - Agar-1.4.1 (Agar版のみ必要。Cocoa版は不要)

    ソースからインストール

    1. パッチを適用します。

           patch -p 1 < agar-1.4.1.patch

    2. ビルド

           ./configure
           make depend
           make
           make install


    * beta版を使用する場合は、--disable-audio, --without-jpeg, 
      --without-png も入れます。
    * config/have_libpng14.hがないといわれたら自分で作る。
      内容は#undef HAVE_LIBPNG14としておく。


  + SDL2の場合

   - SDL2-2.0.8

    ソースからインストール

    1. パッチを適用します。

           patch -p 1 < SDL2-2.0.8-mac-keyboard.patch

    2. ビルド＆インストール

           ./configure
           make
           make install

   - SDL2_ttf-2.0.12

    ソースからインストール

        ./configure
        make
        make install


  + 共通

   - FFmpeg-3.x (libavcodec57, libavutil55, libavformat57, libswscale4)
   (FFmpegを使用しない場合、src/rec_video_defs.hにある #define USE_REC_VIDEO_FFMPEG を
    コメントアウトする。)
    * ヘッダファイルが必要です。
    includeフォルダにヘッダファイルを入れてください。
    * ビルド方法は以下を参考：
     [CompilationGuide/MacOSX – FFmpeg](http://trac.ffmpeg.org/wiki/CompilationGuide/MacOSX/)


  + 以下は任意。通常は不要
   * gettext-0.19.4(libintl)(現バージョンでは使用しません。)
     ソースからインストール(http://www.gnu.org/software/gettext/)

         ./configure
         make
         make install


#### 2. コンパイル（コマンドラインを使用する場合）

 ターミナル上で行います。

 * Makefileの種類

  + SDL1 + Cocoa版 -> Makefile.mac_cocoa, Makefile.mac_cocoa_dbgr
  + SDL2 + Cocoa版 -> Makefile.mac_cocoa2, Makefile.mac_cocoa2_dbgr
  + SDL1 + Agar版  -> Makefile.mac_agar, Makefile.mac_agar_dbgr

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

 Xcodeフォルダにあるプロジェクトを開く。

 * ヘッダファイルやライブラリが/usr/local以外にあるときは
   BuildSettings -> Search Paths にある
   Header Search Paths と Library Search Paths を設定してください。


----------------------------------------
### Linux版 ###

* 以下のバージョンがあります。

 + GTK+3 + SDL1版 -> Makefile.linux_gtk_sdl, Makefile.linux_gtk_sdl_dbgr
 + GTK+3 + SDL2版 -> Makefile.linux_gtk_sdl2
 + SDL1 + GTK+版 -> Makefile.linux_sdl_gtk, Makefile.linux_sdl_gtk_dbgr (不安定)
 + SDL1 + Agar版 -> Makefile.linux_agar

  **SDL1 + GTK+版 と GTK+3 + SDL1版 との違い**

  * 前者はメインウィンドウやイベント処理をSDLで行い、メニュー表示をGTK+で行います。
  * 後者はメインウィンドウやイベント処理をGTK+で行い、画面描画の一部処理をSDLで行います。


#### 1. 開発環境の構築

 * ディストリビューションに付属する開発用ライブラリをインストール。

  + DebianやUbuntu: Synapticパッケージマネージャもしくはapt-getでインストール
  + RedhatやFedora: Yumなどでインストール

  + 必要なライブラリ
   - コンパイラ: gcc, g++, make
   - 画面系: libX11-dev, gtk+, libext-dev, libfreetype-dev, libGL-mesa-dev,
          libsdl-dev, libsdl-ttf-dev
   - サウンド系: libasound2-dev, libpulseaudio-dev などなど
   - 録画用: libavcodec57-dev, libavutil55-dev, libavformat57-dev, libswscale4-dev

 * コンパイルに必要なライブラリをインストールします。

  + SDL1の場合

   - SDL-1.2.15

    付属のパッケージからインストール: libsdl1.2-dev
    or ソースからインストールする場合

        ./configure
        make
        make install

   - SDL_ttf-1.0.20

    付属のパッケージからインストール: libsdl-ttf2.0-dev
    or ソースからインストールする場合

        ./configure
        make
        make install

   - Agar-1.4.1 (Agar版のみ必要。GTK+版は不要)

    ソースからインストール

    1. パッチを適用します。

           patch -p 1 < agar-1.4.1.patch

    2. pthreadがないとエラーになる可能性があるのでスレッドを無効に
     してビルドします。

           ./configure --disable-threads --without-xinerama
           make depend
           make
           make install

    * beta版を使用する場合は、--disable-audio, --without-jpeg, 
      --without-png も入れます。
    * include/agar/config/have_libpng14.hがないといわれたら自分で作る。
      内容は#undef HAVE_LIBPNG14としておく。
    * makeで--tag=CCが不正なオプションとしてエラーになる時は、mk/build.lib.mkを
      編集してLIBTOOLOPTS?=を空にする。


  + SDL2の場合

   - SDL2-2.0.x

    付属のパッケージからインストール。

    ソースからインストールする場合

        ./configure
        make
        make install

   - SDL2_ttf-2.0.x

    付属のパッケージからインストール。

    ソースからインストールする場合

        ./configure
        make
        make install


  + 共通

   * FFmpeg-3.x (libavcodec57, libavutil55, libavformat57, libswscale4)
    (FFmpegを使用しない場合、src/rec_video_defs.hにある #define USE_REC_VIDEO_FFMPEG を
    コメントアウトする。)
    * ヘッダファイルが必要です。
    * 付属のパッケージからインストール。ただし、バージョンが古いと使用できない。

    * ソースからインストールする場合

          ./configure --disable-static --enable-shared --disable-programs
             --disable-doc
          make
          make install


#### 2. コンパイル

 ターミナル(端末)上で行います。

 * Makefileの種類

  + GTK+3 + SDL1版 -> Makefile.linux_gtk_sdl, Makefile.linux_gtk_sdl_dbgr
  + GTK+3 + SDL2版 -> Makefile.linux_gtk_sdl2
  + SDL1 + GTK+版 -> Makefile.linux_sdl_gtk, Makefile.linux_sdl_gtk_dbgr (不安定)
  + SDL1 + Agar版 -> Makefile.linux_agar

 * sharedなバイナリを作成する場合

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
### MinGW + MSYS (Windows)版 ###

* 以下のバージョンがあります。

 + SDL1 + Win GUI版 -> Makefile.win_gui
 + SDL2 + Win GUI版 -> Makefile.win_gui2
 + SDL1 + Agar版  -> Makefile.win_agar

#### 1. 開発環境の構築

 * MinGWをインストール

  インストーラに従ってインストールします。

  + C Compiler, C++ Compiler, MSYS Basic SYSTEM, MSYS Developer Toolkit
  をチェックしてインストール。
    インターネットから必要なモジュールがダウンロードされる。

 * MinGW Shellを起動してコンパイルに必要なライブラリをインストールします。

 + SDL1の場合

  - SDL-1.2.15

    Development Librariesかソースからインストール

    ソースからインストールする場合

        ./configure
        make
        make install

  - freetype-2.4.8

    ソースからインストール

        ./configure
        make
        make install

  - SDL_ttf-1.0.20

    Development Librariesかソースからインストール

    ソースからインストールする場合

        ./configure
        make
        make install

  - Agar-1.4.1 (Agar版のみ必要。Win GUI版は不要)

    ソースからインストール

    1. パッチを適用します。

           patch -p 1 < agar-1.4.1.patch

    2. pthreadがないとエラーになる可能性があるのでスレッドを無効に
       してビルドします。

           ./configure --disable-threads
           make depend
           make
           make install

    * beta版を使用する場合は、--disable-audio, --without-jpeg, 
    --without-png も入れます。
    * config/have_libpng14.hがないといわれたら自分で作る。
      内容は#undef HAVE_LIBPNG14としておく。


 + SDL2の場合

  - SDL2-2.0.x

    Development Librariesかソースからインストール

    ソースからインストールする場合

        ./configure
        make
        make install

  - freetype-2.5.x

    ソースからインストール

        ./configure
        make
        make install

  - SDL2_ttf-2.0.x

    ソースからインストール

        ./configure
        make
        make install


 + 共通

  * FFmpeg-3.x

   (FFmpegを使用しない場合、src/rec_video_defs.hにある #define USE_REC_VIDEO_FFMPEG を
    コメントアウトする。)

    * ヘッダファイルが必要です。
    includeフォルダにヘッダファイルを入れてください。
    * 以下のページから、FFmpeg xxxx 32/64-bit dev をダウンロードすれば、
    ヘッダファイルを入手できます。
    [Zeranoe's FFmpeg Builds Home Page](http://ffmpeg.zeranoe.com/builds/)


  * 以下は任意。通常は不要

   * gettext-0.19.4(libintl)(現バージョンでは使用しません)

    ソースからインストール(http://www.gnu.org/software/gettext/)

        ./configure
        make
        make install


#### 2. コンパイル

 MinGW Shell上で行います。

 + SDL1 + Win GUI版 -> Makefile.win_gui
 + SDL2 + Win GUI版 -> Makefile.win_gui2
 + SDL1 + Agar版  -> Makefile.win_agar

 * 必要に応じてMakefile.xxxを変更します。

  * MinGWのバージョンが異なる場合、GCCLIBDIRを修正。
  * SDLLIBSのパスを修正。
    SDL, SDL_ttfなどをバイナリパッケージでインストールした場合、
    インストール先が/usr/local/cross-tools/i386-mingw32/配下になるため

 * sharedなバイナリを作成する場合

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
### VC++(Windows)版 ###

 * 以下のバージョンがあります。

  + SDL1 + Win GUI版 -> xxxx_sdl.vcxproj
  + SDL2 + Win GUI版 -> xxxx_sdl.vcxproj
  + SDL1 + Agar版  ->xxxx_agar.vcxproj

#### 1. 開発環境の構築

 * MinGW + MSYS (Windows)版に従いAgarまでインストール。

   AgarのビルドでBSDBuildが必要なのでこれをmsysにインストール。
   premakeも入手しておく。

 * 必要なライブラリをVC++でビルドします。

  + SDL1の場合

   - SDL-1.2.15

    バイナリかソースからインストール
    ソースからインストールする場合
      VisualCフォルダにあるSDL.slnを使用してビルド。
      出来たdll,libはlib/Release/x86にコピーしておく。

   - freetype-2.4.8

    ソースからインストール
      builds/win32フォルダにある中から適当に選んでビルド。
      objs/win32にdll,libが出来る。

   - SDL_ttf-1.0.20

    バイナリかソースからインストール
    ソースからインストールする場合
      VisualCフォルダにあるSDL_ttf.slnを使用してビルド。

   - Agar-1.4.1(Agar版のみ必要。)

    ソースからインストール

    1. パッチを適用します。(MinGW上で行う)

           patch -p 1 < agar-1.4.1.patch

    2. vcprojファイルを作成する。(MinGW上で行う)

     * Makefile.projファイルを編集して以下の行を追加。

        PROJFILES=windows:vs2005:-nothreads:--disable-threads
        元の行は#でコメントアウト。
        * beta版を使用する場合は、--disable-audio, --without-jpeg, 
        --without-png も入れます。

     * make projを実行。(premake.exe, zip.exeなどが必要。)

        成功するとProjectFilesフォルダにvs2005-windows-notreads.zipが出来る。

     * zipファイルを展開する。

        カレントフォルダに展開する。各フォルダにvcprojファイルが展開される。

    3. ビルド

     * Agar.slnを使用してビルド。

      + プロジェクト→プロパティを開き、C++の追加のインクルードディレクトリに
        上記SDLなどのincludeへのパスを追加する。
       - 最初に$(VCInstallDir)includeを追加したほうがいいかも。
      + 同様にリンカの追加のライブラリディレクトリにパスを追加する。
      + 同様にリンカの追加の依存ファイルにSDL.libなどのファイル名を追加する。
       - core, gui だけでok.


 + SDL2の場合

  - SDL2-2.0.x

    ソースからインストール
      VisualCフォルダにあるSDL.slnを使用してビルド。
      出来たdll,libはlib/Release/x86にコピーしておく。

  - freetype-2.5.x

    ソースからインストール
      builds/win32フォルダにある中から適当に選んでビルド。
      objs/win32にdll,libが出来る。

  - SDL2_ttf-2.0.x

    ソースからインストール
      VisualCフォルダにあるSDL_ttf.slnを使用してビルド。


 + 共通

  - FFmpeg-3.x
   (FFmpegを使用しない場合、src/rec_video_defs.hにある #define USE_REC_VIDEO_FFMPEG を
    コメントアウトする。)
    * ヘッダファイルが必要です。
    includeフォルダにヘッダファイルを入れてください。
    * 以下のページから、FFmpeg xxxx 32/64-bit dev をダウンロードすれば、
    ヘッダファイルを入手できます。
    [Zeranoe's FFmpeg Builds Home Page](http://ffmpeg.zeranoe.com/builds/)


 + 以下は任意。通常は不要

  * gettext-0.19.4(libintl)(現バージョンでは使用しません)
   + ソースを入手
     http://www.gnu.org/software/gettext/
   + 以下からプロジェクトファイルを入手してビルドしてください。
     http://osdn.jp/projects/libintl-msvc10/
   + libフォルダに作成したlibファイルを入れてください。

#### 2. コンパイル

 * プロジェクトの種類

  + SDL1 + Win GUI版 -> xxxx_sdl.vcxproj
  + SDL2 + Win GUI版 -> xxxx_sdl.vcxproj
  + SDL1 + Agar版  ->xxxx_agar.vcxproj

 * プロジェクトファイルを使用してビルド。
   表示→プロパティマネージャを開き、Release下を開く。
   ユーザーマクロに設定しているパスを変更する。


----------------------------------------
### FreeBSD版 ###

#### 1. 開発環境の構築

  Xorg, GNOMEなどをpackageやportsからインストールしておきます。

  インストールはlinux版を参考にして下さい。

#### 2. コンパイル

  GNU make (gmake) を使用してください。

  make方法はlinux版を参考にして下さい。




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

