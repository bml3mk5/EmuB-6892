# HITACHI BASIC MASTER LEVEL3 MARK5 Emulator
#       wxWidgets edition

#### Copyright(C) Common Source Code Project, Sasaji 2012-2022 All Rights Reserved.

## ファイル構成

    docs/ .................. ドキュメント
    font/ .................. 画面フォント作成perlスクリプト
      kanji/ ............... 擬似漢字ROM作成プログラム
    source/
      data/ ................ データファイル
      locale/ .............. 翻訳用(gettext)
        list.xml ........... 言語一覧
        ja/
          LC_MESSAGES/
            bml3mk5.po ..... 日本語翻訳ファイル
            bml3mk5.mo ..... 日本語翻訳ファイル(変換済み)
      src/ ................. ソースファイル
        extra/ ............. その他ソース
        gui/ ............... GUI関連ソース
          wxwidgets/ ....... wxWidgets GUI関連ソース
        osd/ ............... OS依存関連ソース
          SDL/ ............. SDL依存関連ソース
          wxwidgets/ ....... wxWidgets依存関連ソース
        res/ ............... リソースファイル
        video/ ............. 録画用関連ソース
          avkit/ ........... AVkit(mac用)関連ソース
          ffmpeg/ .......... ffmpeg関連ソース
          qtkit/ ........... qtkit(mac用)関連ソース
          vfw/ ............. video for windows関連ソース
          wave/ ............ waveフォーマット関連ソース
          wxwidgets/ ....... wxWidgets関連ソース
        vm/ ................ VMメインプログラムソース
      Eclipse/ ............. Eclipseプロジェクトファイル
        wx_linux/ .......... wxWidgets版 linux用
        wx_win/ ............ wxWidgets版 Pleiades(Eclipse日本語版)用   
      VC++2010/
        bml3mk5_wx.vcxproj .. wxWidgets版 VC++2010用プロジェクトファイル
      Xcode/ ............... Xcode用プロジェクトファイル
      Makefile.xxx ......... 各OSごとのmakeファイル
      README_WX.md ......... このファイル


## 必要なバージョン

 wxWidgets 3.0以上


## コンパイル方法

### MacOSX版 ###

* 以下のバージョンがあります。

 + wxWidgets3 + SDL1版 -> Makefile.mac_wx
 + wxWidgets3 + SDL2版 -> Makefile.mac_wx2

  * SDLはサウンド処理で使用します。


#### 1. 開発環境の構築

 * Xcode を インストールします。
   (Command Line Tools for Xcode は自動的にインストールされるようだ。)

 * コンパイルに必要なライブラリをインストールします。
   ターミナル上で行います。(Xcodeは使用しません。)

  + wxWidgets-3.1.0

    ソースからインストールする場合

    * staticライブラリにする場合

          mkdir build_release_static_unicode
          cd build_release_static_unicode
          ../configure --with-osx_cocoa --disable-debug --disable-shared --enable-unicode
          make

    * sharedライブラリにする場合

          mkdir build_release_shared_unicode
          cd build_release_shared_unicode
          ../configure --with-osx_cocoa --disable-debug --enable-shared --enable-unicode
          make

  + SDL1の場合

   - SDL-1.2.15

    ソースからインストール

        ./configure
        make
        make install

  + SDL2の場合

   - SDL2-2.0.8

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
     [CompilationGuide/MacOSX ? FFmpeg](http://trac.ffmpeg.org/wiki/CompilationGuide/MacOSX/)


#### 2. コンパイル（コマンドラインを使用する場合）

 ターミナル上で行います。

 * Makefileの種類

  + wxWidgets3 + SDL1版 -> Makefile.mac_wx
  + wxWidgets3 + SDL2版 -> Makefile.mac_wx2

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

 + wxWidgets3 + SDL1版 -> Makefile.linux_wx
 + wxWidgets3 + SDL2版 -> Makefile.linux_wx2

  * SDLはサウンド処理で使用します。


#### 1. 開発環境の構築

 * ディストリビューションに付属する開発用ライブラリをインストール。

  + DebianやUbuntu: Synapticパッケージマネージャもしくはapt-getでインストール
  + RedhatやFedora: Yumなどでインストール

  + 必要なライブラリ
   - コンパイラ: gcc, g++, make
   - 画面系: libwxbase3.1-dev, libwxgtk3.1-gtk3-dev, libsdl-dev
   - 録画用: libavcodec57-dev, libavutil55-dev, libavformat57-dev, libswscale4-dev

 * コンパイルに必要なライブラリをインストールします。

  + wxWidgets-3.0.x or later
  
   パッケージから：libwxgtk3-dev

    ソースからインストールする場合

    * staticライブラリにする場合

          mkdir build_release_static_unicode
          cd build_release_static_unicode
          ../configure --with-gtk --disable-debug --disable-shared --enable-unicode
          make

    * sharedライブラリにする場合

          mkdir build_release_shared_unicode
          cd build_release_shared_unicode
          ../configure --with-gtk --disable-debug --enable-shared --enable-unicode
          make


  + SDL1の場合

   - SDL-1.2.15
    付属のパッケージからインストール。

    ソースからインストールする場合

        ./configure
        make
        make install


  + SDL2の場合

   - SDL2-2.0.8
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

          ./configure --disable-static --enable-shared --disable-programs --disable-doc
          make
          make install


#### 2. コンパイル

 ターミナル(端末)上で行います。

 * Makefileの種類

  + wxWidgets3 + SDL1版 -> Makefile.linux_wx
  + wxWidgets3 + SDL2版 -> Makefile.linux_wx2

 * sharedなバイナリを作成する場合

       make -f Makefile.xxx clean
       make -f Makefile.xxx install

  * installは、makeを実行したディレクトリの上にReleaseディレクトリを作成し、
    そこに必要なファイルをコピーします。
  (/usr/localにコピーはしません。)
  * インストール先を変更するには、Makefile中にある、SH_INSTALLDIRを変更
    してください。

 * staticなバイナリを作成する場合

   Makefile内のWXDIRをwxWidgets3.1をビルドしたディレクトリにして下さい。

       make -f Makefile.xxx st_clean
       make -f Makefile.xxx st_install

  * インストール先を変更するには、Makefile中にある、ST_INSTALLDIRを変更
    してください。


----------------------------------------
### MinGW + MSYS (Windows)版 ###

* 以下のバージョンがあります。

 + wxWidgets3 + SDL1版 -> Makefile.win_wx
 + wxWidgets3 + SDL2版 -> Makefile.win_wx2

  * SDLはサウンド処理で使用します。

#### 1. 開発環境の構築

 * MinGWをインストール

  インストーラに従ってインストールします。

  + C Compiler, C++ Compiler, MSYS Basic SYSTEM, MSYS Developer Toolkit
  をチェックしてインストール。
    インターネットから必要なモジュールがダウンロードされる。

 * MinGW Shellを起動してコンパイルに必要なライブラリをインストールします。

 + wxWidgets-3.1.0

    ソースからインストールする場合

    * staticライブラリにする場合

          mkdir build_release_static_unicode
          cd build_release_static_unicode
          ../configure --with-msw --disable-debug --disable-shared --enable-unicode
          make

    * sharedライブラリにする場合

          mkdir build_release_shared_unicode
          cd build_release_shared_unicode
          ../configure --with-msw --disable-debug --enable-shared --enable-unicode
          make 

 + SDL1の場合

  - SDL-1.2.15

    Development Librariesかソースからインストール

    ソースからインストールする場合

        ./configure
        make
        make install


 + SDL2の場合

  - SDL-2.0.x

    Development Librariesかソースからインストール
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


#### 2. コンパイル

 MinGW Shell上で行います。

 + wxWidgets3 + SDL1版 -> Makefile.win_wx
 + wxWidgets3 + SDL2版 -> Makefile.win_wx2

 * 必要に応じてMakefile.xxxを変更します。

  * MinGWのバージョンが異なる場合、GCCLIBDIRを修正。
  * WXDIRを編集。
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

  + wxWidgets3 + SDL版 -> xxxx_wx.vcxproj

#### 1. 開発環境の構築

* 必要なライブラリをVC++でビルドします。

 + wxWidgets-3.1.0

  - wxWidgets-3.1.0.zipをダウンロードして適当なフォルダに展開。
  - build\mswにあるwx_vc??.slnをVC++で開く。
  - Debug/Releaseでソリューションをビルドすると、lib\vc_lib\に
    ライブラリが生成される。

  + SDL1の場合

   - SDL-1.2.15

    バイナリかソースからインストール
    ソースからインストールする場合
      VisualCフォルダにあるSDL.slnを使用してビルド。
      出来たdll,libはlib/Release/x86にコピーしておく。

  + SDL2の場合

  - SDL2-2.0.x

    ソースからインストール
      VisualCフォルダにあるSDL.slnを使用してビルド。
      出来たdll,libはlib/Release/x86にコピーしておく。

 + 共通

  - FFmpeg-3.x
   (FFmpegを使用しない場合、src/rec_video_defs.hにある #define USE_REC_VIDEO_FFMPEG を
    コメントアウトする。)
    * ヘッダファイルが必要です。
    includeフォルダにヘッダファイルを入れてください。
    * 以下のページから、FFmpeg xxxx 32/64-bit dev をダウンロードすれば、
    ヘッダファイルを入手できます。
    [Zeranoe's FFmpeg Builds Home Page](http://ffmpeg.zeranoe.com/builds/)


#### 2. コンパイル

 * wxWidgets3 + SDL版 -> xxxx_wx.vcxproj

  bml3mk5_wx.vcprojを使用してビルド。
    表示→プロパティマネージャを開き、Release下にあるbml3mk5を開く。
    ユーザーマクロに設定しているパスを変更する。


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

