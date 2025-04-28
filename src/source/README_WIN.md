# HITACHI BASIC MASTER LEVEL3 MARK5 Emulator Windows(VC++) Edition

#### Copyright(C) Common Source Code Project, Sasaji 2011-2025 All Rights Reserved.

## ファイル構成

    docs/ .................. ドキュメント
    font/ .................. 画面フォント作成スクリプト
      kanji/ ............... 擬似漢字ROM作成ツール
        Win/ ............... 擬似漢字ROM作成プログラム(WinAPI使用)
    source/
      include/ ............. インクルードファイル
      lib/ ................. ライブラリファイル
      locale/ .............. 翻訳用(gettext)
        list.xml ........... 言語一覧
        ja/
          LC_MESSAGES/
            bml3mk5.po ..... 日本語翻訳ファイル
            bml3mk5.mo ..... 日本語翻訳ファイル(変換済み)
      src/ ................. ソースファイル
        gui/ ............... GUI関連ソース
          windows/ ......... Windows GUI関連ソース
        osd/ ............... OS依存関連ソース
          windows/ ......... Windows依存関連ソース
        res/ ............... リソースファイル
        video/ ............. 録画用関連ソース
          ffmpeg/ .......... ffmpeg関連ソース
          mmf/ ............. media foundation (windows)関連ソース
          vfw/ ............. video for windows関連ソース
          wave/ ............ waveフォーマット関連ソース
          windows/ ......... windows関連ソース
        vm/ ................ VMメインプログラムソース
      VC++2010/
        bml3mk5.vcxproj ...... Visual Studio プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      VC++2013/
        bml3mk5.vcxproj ...... Visual Studio プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      VC++2015/
        bml3mk5.vcxproj ...... Visual Studio プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      VC++2017/
        bml3mk5.vcxproj ...... Visual Studio プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      VC++2019/
        bml3mk5.vcxproj ...... Visual Studio プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      README_WIN.md ........ このファイル

## コンパイル方法

  Visual Studioからvcxprojファイルを開いてビルドしてください。

* 必要なライブラリ

  * FFmpeg-4.x (https://ffmpeg.org/)

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

* mmf_loader
  * Microsoft Media Foundationを使用するためのDLLファイルを作成します。



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

