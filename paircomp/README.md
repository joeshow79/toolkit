[USAGE]
usage: paircomp -f fileList -k [Repeat Times] -r [Random Ratio]

-f: List file.
-k: Repeat times. Must be integer and greater than 0
-r: Random ratio. Must be the value between (0.0,1.0]

[KEYS]
选择：1 或者 2
缩放： +  -
平移：上下左右
复位：r
下一张：Enter
退出: ESC

[LIST FILE EXAMPLE]
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_A.jpg /Users/jasonjmac/Downloads/打分数据/S01_B.jpg
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_A.jpg /Users/jasonjmac/Downloads/打分数据/S01_C.jpg
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_A.jpg /Users/jasonjmac/Downloads/打分数据/S01_D.jpg
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_B.jpg /Users/jasonjmac/Downloads/打分数据/S01_C.jpg
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_B.jpg /Users/jasonjmac/Downloads/打分数据/S01_D.jpg
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_C.jpg /Users/jasonjmac/Downloads/打分数据/S01_D.jpg
Border /Users/jasonjmac/Downloads/打分数据/S12_A.jpg /Users/jasonjmac/Downloads/打分数据/S12_B.jpg
Border /Users/jasonjmac/Downloads/打分数据/S12_A.jpg /Users/jasonjmac/Downloads/打分数据/S12_C.jpg
Border /Users/jasonjmac/Downloads/打分数据/S12_A.jpg /Users/jasonjmac/Downloads/打分数据/S12_D.jpg
Border /Users/jasonjmac/Downloads/打分数据/S12_B.jpg /Users/jasonjmac/Downloads/打分数据/S12_C.jpg
Border /Users/jasonjmac/Downloads/打分数据/S12_B.jpg /Users/jasonjmac/Downloads/打分数据/S12_D.jpg
Border /Users/jasonjmac/Downloads/打分数据/S12_C.jpg /Users/jasonjmac/Downloads/打分数据/S12_D.jpg

[RESULT FILE EXAMPLE]
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_A.jpg /Users/jasonjmac/Downloads/打分数据/S01_B.jpg 1
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_A.jpg /Users/jasonjmac/Downloads/打分数据/S01_C.jpg 2
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_A.jpg /Users/jasonjmac/Downloads/打分数据/S01_D.jpg 1
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_B.jpg /Users/jasonjmac/Downloads/打分数据/S01_C.jpg 2
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_B.jpg /Users/jasonjmac/Downloads/打分数据/S01_D.jpg 2
OutFocus /Users/jasonjmac/Downloads/打分数据/S01_C.jpg /Users/jasonjmac/Downloads/打分数据/S01_D.jpg 2
Border /Users/jasonjmac/Downloads/打分数据/S12_A.jpg /Users/jasonjmac/Downloads/打分数据/S12_B.jpg 2
Border /Users/jasonjmac/Downloads/打分数据/S12_A.jpg /Users/jasonjmac/Downloads/打分数据/S12_C.jpg 2
Border /Users/jasonjmac/Downloads/打分数据/S12_A.jpg /Users/jasonjmac/Downloads/打分数据/S12_D.jpg 1
Border /Users/jasonjmac/Downloads/打分数据/S12_B.jpg /Users/jasonjmac/Downloads/打分数据/S12_C.jpg 2
Border /Users/jasonjmac/Downloads/打分数据/S12_B.jpg /Users/jasonjmac/Downloads/打分数据/S12_D.jpg 1
