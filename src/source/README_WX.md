# HITACHI BASIC MASTER LEVEL3 MARK5 Emulator wxWidgets edition

#### Copyright(C) Common Source Code Project, Sasaji 2012-2025 All Rights Reserved.

## ファイル構成

    docs/ .................. ドキュメント
    font/ .................. 画面フォント作成スクリプト
      kanji/ ............... 擬似漢字ROM作成ツール
        SDL/ ............... 擬似漢字ROM作成プログラム(SDL使用)
        Win/ ............... 擬似漢字ROM作成プログラム(WinAPI使用)
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
        bml3mk5_wx.vcxproj .. wxWidgets版 Visual Studio プロジェクトファイル
      VC++2013/
        bml3mk5_wx.vcxproj .. wxWidgets版 Visual Studio プロジェクトファイル
      VC++2015/
        bml3mk5_wx.vcxproj .. wxWidgets版 Visual Studio プロジェクトファイル
      VC++2017/
        bml3mk5_wx.vcxproj .. wxWidgets版 Visual Studio プロジェクトファイル
      VC++2019/
        bml3mk5_wx.vcxproj .. wxWidgets版 Visual Studio プロジェクトファイル
      Xcode/ ............... Xcode用プロジェクトファイル
      Makefile.xxx ......... 各OSごとのmakeファイル
      README_WX.md ......... このファイル


## 必要なバージョン

 wxWidgets 3.0以上


## コンパイル方法


----------------------------------------
### MacOS版 ###

* 以下のバージョンがあります。

  + wxWidgets3 + SDL2版 -> Makefile.mac_wx2, kefile.mac_wx2_dbgr

  * 上記以外のMakefileは現在メンテナンスしていません。
  * SDLはサウンド処理で使用します。


#### 1. 開発環境の構築

 * Xcode を インストールします。
   (Command Line Tools for Xcode は自動的にインストールされるようだ。)

 * コンパイルに必要なライブラリをインストールします。
   ターミナル上で行います。(Xcodeは使用しません。)

   + ソースコードからビルドする場合

     - wxWidgets-3.2.4

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

     - SDL2-2.28.5

           ./configure
           make
           make install

     - FFmpeg-6.1.1 (https://ffmpeg.org/)

       (FFmpegを使用しない場合、src/rec_video_defs.hにある \#define USE_REC_VIDEO_FFMPEG を
       コメントアウトする。)

       * ヘッダファイルが必要です。
         includeフォルダにヘッダファイルを入れてください。

       * ビルド方法は以下を参考：
         [CompilationGuide/MacOSX - FFmpeg](http://trac.ffmpeg.org/wiki/CompilationGuide/MacOSX/)


   + Homebrewを使用する場合

     - sdl2, wxwidgets, ffmpeg@6 をインストールしてください。

#### 2. コンパイル（コマンドラインを使用する場合）

 ターミナル上で行います。

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

  + wxWidgets3 + SDL2版 -> Makefile.linux_wx2, Makefile.linux_wx2_dbgr

  * 上記以外のMakefileは現在メンテナンスしていません。
  * SDLはサウンド処理で使用します。


#### 1. 開発環境の構築

 * ディストリビューションに付属する開発用ライブラリをインストール。

   + DebianやUbuntu: Synapticパッケージマネージャもしくはaptでインストール
   + RedhatやFedora: Yumなどでインストール

   + 必要なライブラリ
     - コンパイラ: gcc, g++, make
     - 画面系: libwxbase3.2-dev, libwxgtk3.2-gtk3-dev, libsdl2-dev
     - 録画用: libavcodec60-dev, libavutil58-dev, libavformat60-dev, libswscale7-dev
       (これらはffmpeg6のライブラリ)

 * コンパイルに必要なライブラリをインストールします。

   + wxWidgets-3.2.x
  
     - パッケージから：libwxgtk3-dev

     - ソースからインストールする場合

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

   + SDL2-2.28.5
     - 付属のパッケージからインストール。

     - ソースからインストールする場合

           ./configure
           make
           make install

   + FFmpeg-4.x (https://ffmpeg.org/)

     (FFmpegを使用しない場合、src/rec_video_defs.hにある \#define USE_REC_VIDEO_FFMPEG を
     コメントアウトする。)

     * ヘッダファイルが必要です。
     * 付属のパッケージからインストール。ただし、バージョンが古いと使用できない。

     * ソースからインストールする場合

           ./configure --disable-static --enable-shared --disable-programs --disable-doc
           make
           make install


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

   - Makefile内のWXDIRをwxWidgets3.1をビルドしたディレクトリにして下さい。

         make -f Makefile.xxx st_clean
         make -f Makefile.xxx st_install

   * インストール先を変更するには、Makefile中にある、ST_INSTALLDIRを変更
     してください。


----------------------------------------
### MinGW + MSYS (Windows)版 ###

* 以下のバージョンがあります。

  + wxWidgets3 + SDL2版 -> Makefile.win_wx2, Makefile.win_wx2_dbgr

  * 上記以外のMakefileは現在メンテナンスしていません。
  * SDLはサウンド処理で使用します。

#### 1. 開発環境の構築

 * MinGWをインストール

   + インストーラに従ってインストールします。

   + C Compiler, C++ Compiler, MSYS Basic SYSTEM, MSYS Developer Toolkit
     をチェックしてインストール。
     インターネットから必要なモジュールがダウンロードされる。

 * MinGW Shellを起動してコンパイルに必要なライブラリをインストールします。

   + wxWidgets-3.1.0

     - ソースからインストールする場合

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

   + SDL2-2.28.5

     - Development Librariesかソースからインストール

     - ソースからインストールする場合

           ./configure
           make
           make install

   + FFmpeg-6.x (https://ffmpeg.org/)

     (FFmpegを使用しない場合、src/rec_video_defs.hにある \#define USE_REC_VIDEO_FFMPEG を
     コメントアウトする。)

     * ヘッダファイルが必要です。
       includeフォルダにヘッダファイルを入れてください。


#### 2. コンパイル

 MinGW Shell上で行います。

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

#### 1. 開発環境の構築

* 必要なライブラリをVC++でビルドします。

  + wxWidgets-3.2.4

    - wxWidgets-3.2.4.zipをダウンロードして適当なフォルダに展開。
    - build\\mswにあるwx_vc??.slnをVC++で開く。
    - Debug/Releaseでソリューションをビルドすると、lib\\vc_lib\\に
      ライブラリが生成される。

  + SDL2-2.28.5

    ソースからインストール
      VisualCフォルダにあるSDL.slnを使用してビルド。
      出来たdll,libはlib/Release/x86にコピーしておく。

  + FFmpeg-6.1.1 (https://ffmpeg.org/)

   (FFmpegを使用しない場合、src/rec_video_defs.hにある \#define USE_REC_VIDEO_FFMPEG を
    コメントアウトする。)

    * ヘッダファイルが必要です。
      includeフォルダにヘッダファイルを入れてください。
    * 64ビット版のsharedライブラリとincludeファイルは以下のサイトから入手できます。
      [CODEX FFMPEG](https://www.gyan.dev/ffmpeg/builds/)
      にある ffmpeg-6.1.1-full_build-shared.7z をダウンロードします。
    * 上記にない場合はGithub内の以下にあります。
      [CODEX FFMPEG Github](https://github.com/GyanD/codexffmpeg/releases/tag/6.1.1)
    * ヘッダファイルがあれば、32ビット版のビルドもできます。
    * バージョン5のヘッダファイルでもビルドできます。
    * バージョン4のヘッダファイルでもビルドできますが動作するかは未確認。

#### 2. コンパイル

 * プロジェクトファイル(*_wx.vcxproj)を使用してビルド。
   + 表示→プロパティマネージャを開き、Release下を開く。
   + ユーザーマクロに設定しているパスを変更する。


----------------------------------------
## 免責事項

* このソフトはフリーウェアです。ただし、著作権は放棄しておりません。
  実行モジュールについては作者Sasajiにあります。
  ソースコードについてはそれぞれの作者にあります。
* このソフトによって発生したいかなる損害についても著作権者は一切責任を負いません。
  このソフトを使用するにあたってはすべて自己責任で行ってください。
* 雑誌やネットなどに転載される場合、不特定多数の方に再配布を行う場合でも
  承諾の必要はありませんが、転載の旨をご連絡いただけたら幸いです。

------------------------------------------------------------------------------

連絡先：Sasaji (sasaji@s-sasaji.ddo.jp)
 * My WebPage: http://s-sasaji.ddo.jp/bml3mk5/
 * GitHub:     https://github.com/bml3mk5/EmuB-6892
 * X(Twitter): https://x.com/bml3mk5

------------------------------------------------------------------------------

