# mkkanji - Make Kanji ROM image from Kanji font

#### Copyright(C) Common Source Code Project, Sasaji All Rights Reserved.

----------------------------------------

#### 1. 開発環境の構築

* 必要なライブラリをVC++でビルドします。

  + SDL2-2.28.5

    - ソースからインストール
      VisualCフォルダにあるSDL.slnを使用してビルド。
      出来たdll,libはlib10/Win32/Releaseにコピーしておく。

  + SDL2_ttf-2.20.2

    - ソースからインストール
      VisualCフォルダにあるSDL_ttf.slnを使用してビルド。
      出来たdll,libはlib10/Win32/Releaseにコピーしておく。

#### 2. コンパイル

 * プロジェクトファイル(*.vcxproj)を使用してビルド。
   + メニューバーから表示→プロパティマネージャを開く。
   + マネージャの"Release | Win32"にあるlibrary_sdlを開く。
   + ユーザーマクロに設定している以下のパスを上記でインストールしたフォルダに
     する。
     - DevelopDir
     - SDL2Dir
     - SDL2TTfDir
 * "Release | Win32"でビルド。プロジェクトフォルダにReleaseフォルダができる。

#### 3. 実行

 * Releaseフォルダに、kanji.txt, kanji2.txt, ipag.ttfをコピーする。
 * mkkanji.exeを実行する。
 * KANJI.ROMが作成される。

----------------------------------------

#### 1. Build libraries

* Build the libraries using VC++.

  + SDL2-2.28.5

    - Install from sources
      Build the library using SDL.sln in VisualC folder.
      Copy the built dll and lib to lib10/Win32/Release folder.

  + SDL2_ttf-2.20.2

    - Install from sources
      Build the library using SDL_ttf.sln in VisualC folder.
      Copy the built dll and lib to lib10/Win32/Release folder.

#### 2. Build the mkkanji

 * Build the executable using the project file (*.vcxproj).
   + Open View -> Property Manager on the menu bar.
   + Open library_sdl on the "Release | Win32" folder in the manager.
   + Change the following path in the user macro to the folder you installed.
     - DevelopDir
     - SDL2Dir
     - SDL2TTfDir
 * Build it on "Release | Win32". Created Release folder on the project folder.

#### 3. Run

 * Copy the kanji.txt, kanji2.txt and ipag.ttf to Release folder.
 * Run mkkanji.exe
 * KANJI.ROM is created on current folder.
