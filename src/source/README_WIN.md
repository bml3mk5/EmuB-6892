# HITACHI BASIC MASTER LEVEL3 MARK5 Emulator Windows(VC++) Edition

#### Copyright(C) Common Source Code Project, Sasaji 2011-2024 All Rights Reserved.

## ファイル構成

    docs/ .................. ドキュメント
    font/ .................. 画面フォント作成perlスクリプト
      kanji/ ............... 擬似漢字ROM作成プログラム
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
        bml3mk5.vcxproj ...... VC++2010用プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      VC++2013/
        bml3mk5.vcxproj ...... VC++2013用プロジェクトファイル
        mmf_loader.vcxproj ... Media Foundationを動的にロードするためのDLL作成
      VC++2015/
        bml3mk5.vcxproj ...... VC++2015用プロジェクトファイル
      VC++2017/
        bml3mk5.vcxproj ...... VC++2017用プロジェクトファイル
      VC++2019/
        bml3mk5.vcxproj ...... VC++2019用プロジェクトファイル
      README_WIN.md ........ このファイル

## コンパイル方法

* 必要なライブラリ
  * FFmpeg-3.x
   (FFmpegを使用しない場合、src/rec_video_defs.hにある #define USE_REC_VIDEO_FFMPEG を
    コメントアウトする。)
    * ヘッダファイルが必要です。
    includeフォルダにヘッダファイルを入れてください。

* mmf_loader
  * Microsoft Media Foundationを使用するためのDLLファイルを作成します。

* その他のライブラリ(現バージョンでは使用しません)
  * gettext-0.19.4(libintl)
    1. ソースを入手(http://www.gnu.org/software/gettext/)
    2. 以下からプロジェクトファイルを入手してビルドしてください。
       (http://osdn.jp/projects/libintl-msvc10/)
    3. libフォルダに作成したlibファイルを入れてください。


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

