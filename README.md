# ccvt
chinese converter for utf-8 encoded text files

## 簡介

ccvt 是一個命令列程式，可以將 UTF8 格式的輸入，通過設定的字詞代換模式，處理後輸出。內建並預設的模式為簡體中文轉換為繁體中文，另外也內建繁體中文轉換為簡體的模式。

## 使用

    ccvt [-h] [-c] [-m convert_mode] [ files... ]

- -h 顯示 help 訊息
- -c 使用 console 輸入
- -m 使用不同轉換模式，預設為 tc 模式即簡轉繁，另外有 sc 模式為繁轉簡

## 擴展

- 資料目錄下的子目錄即為模式名稱，可以自行建立新模式

- 預設的資料目錄在 Unix 下為 /usr/local/share/ccvt，在 Windows 下為 C:\Program Files\ccvt

- 模式目錄裡必須要有 ccvt.dat 檔，這是個別 UTF8 字元轉換的資料

- skip.txt 為不做字詞轉換的 UTF8 字元，適當設定可以加快轉換效率，例如設定 “，” 會讓逗號前幾個字的詞彙檢查長度縮短

- 副檔名為 .conf 的檔案都是詞彙轉換資料，可自行添加或修改

- 目前有 moe_dict.conf 為辭典摘選而來的詞彙，correct.conf 用作較長詞彙的修正，但基本上是空的，只有兩個範例，請自行添加

- 可轉換詞彙最長為 16 個 UTF8 字元

## 算法

1. 先使用 ccvt.dat 的資料做單字轉換
2. 再使用 *.conf 的資料做詞彙轉換，以較長詞彙優先，只轉換一次，遇到 skip.txt 裡的字元就不處理

## 範例

- 單字轉換 准 -> 準 (ccvt.dat)
- 詞彙轉換 不準 -> 不准 (moe_dict.conf)
- 詞彙轉換 測不準 -> 測不準 (correct.conf)

## 資料來源

中華人民共和國教育部 國家語言文字工作委員會 通用規範漢字表
http://www.gov.cn/gzdt/att/att/site1/20130819/tygfhzb.pdf \
https://zh.wikisource.org/wiki/通用规范汉字表

- 繁體「别」字改為「別」
- 繁體「説」字改為「說」
- 已有「赀」可用，刪除「资」與「貲」的簡繁對應

中華民國教育部終身教育司 常用國字標準字體表 \
https://zh.wikisource.org/wiki/常用國字標準字體表

- 通用規範漢字表的簡繁對應並不完整，還要將其中的簡體字與異體字，
 對照常用國字標準字體表來補充，才算是得到了規範化的簡繁對應表
- 增加「着」與「著」的簡繁對應
- 簡繁字多對一轉換，以常用單字詞、姓氏、常用字較優先，其他則以簡繁同字、詞彙數量多者為主
- 簡轉繁 2581 字，繁轉簡 2762 字

教育部重編國語辭典修訂本 \
http://dict.revised.moe.edu.tw/cbdic/

- 有逗號的成語和完全重疊的詞彙簡化收錄
- 日常用語的「臺」「帘」「晒」字詞不收錄
- 「祇是」不常用，改成「祇是當時已惘然」
- 「癥狀」不常用，不收錄
- 部份「慾」字詞不常用，不收錄
- 共收錄簡轉繁調整詞彙 3777 組

## 關於原始碼

只有 main.c ccvt.c ccvt.h 是主要的程式碼，其餘是個人的程式庫內容，但若是分開來要寫 CMakeLists.txt 會有些複雜，乾脆全部放在一起。
