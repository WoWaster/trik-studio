// return data in format ARRAY32 5 [C,R,G,B,W] 0-17, 0-255
ARRAY8 ar@@RANDOM_ID_1@@ 4
ARRAY(CREATE8, 4, ar@@RANDOM_ID_1@@)

//enables lights
ARRAY_WRITE(ar@@RANDOM_ID_1@@, 0, 1)
ARRAY_WRITE(ar@@RANDOM_ID_1@@, 1, 65)
ARRAY_WRITE(ar@@RANDOM_ID_1@@, 2, 0)
INPUT_DEVICE(SETUP, 0, @@PORT@@, 1, 0, 3, @ar@@RANDOM_ID_1@@, 0, @ar@@RANDOM_ID_1@@)
TIMER_WAIT(110, timer)
TIMER_READY(timer)

ARRAY_WRITE(ar@@RANDOM_ID_1@@, 0, 1)
ARRAY_WRITE(ar@@RANDOM_ID_1@@, 1, 66)

ARRAY8 answer@@RANDOM_ID_2@@ 8
ARRAY(CREATE8, 8, answer@@RANDOM_ID_2@@)

ARRAY32 answerALL@@RANDOM_ID_3@@ 8
ARRAY(CREATE32, 8, answerALL@@RANDOM_ID_3@@)
DATA8 tmp@@RANDOM_ID_4@@
DATA32 tmp32@@RANDOM_ID_5@@

INPUT_DEVICE(SETUP, 0, @@PORT@@, 1, 0, 2, @ar@@RANDOM_ID_1@@, 5, @answer@@RANDOM_ID_2@@)

ARRAY_READ(answer@@RANDOM_ID_2@@, 0, tmp@@RANDOM_ID_4@@)
MOVE8_32(tmp@@RANDOM_ID_4@@, tmp32@@RANDOM_ID_5@@)
ARRAY_WRITE(answerALL@@RANDOM_ID_3@@, 4, tmp32@@RANDOM_ID_5@@)
JR_GTEQ8(tmp@@RANDOM_ID_4@@, 0, m@@RANDOM_ID_6@@)
AND8(tmp@@RANDOM_ID_4@@, 127, tmp@@RANDOM_ID_4@@)
MOVE8_32(tmp@@RANDOM_ID_4@@, tmp32@@RANDOM_ID_5@@)
ADD32(tmp32@@RANDOM_ID_5@@, 128, tmp32@@RANDOM_ID_5@@)
ARRAY_WRITE(answerALL@@RANDOM_ID_3@@, 4, tmp32@@RANDOM_ID_5@@)

m@@RANDOM_ID_6@@:
ARRAY_READ(answer@@RANDOM_ID_2@@, 1, tmp@@RANDOM_ID_4@@)
MOVE8_32(tmp@@RANDOM_ID_4@@, tmp32@@RANDOM_ID_5@@)
ARRAY_WRITE(answerALL@@RANDOM_ID_3@@, 3, tmp32@@RANDOM_ID_5@@)
JR_GTEQ8(tmp@@RANDOM_ID_4@@, 0, m@@RANDOM_ID_7@@)
AND8(tmp@@RANDOM_ID_4@@, 127, tmp@@RANDOM_ID_4@@)
MOVE8_32(tmp@@RANDOM_ID_4@@, tmp32@@RANDOM_ID_5@@)
ADD32(tmp32@@RANDOM_ID_5@@, 128, tmp32@@RANDOM_ID_5@@)
ARRAY_WRITE(answerALL@@RANDOM_ID_3@@, 3, tmp32@@RANDOM_ID_5@@)

m@@RANDOM_ID_7@@:
ARRAY_READ(answer@@RANDOM_ID_2@@, 2, tmp@@RANDOM_ID_4@@)
MOVE8_32(tmp@@RANDOM_ID_4@@, tmp32@@RANDOM_ID_5@@)
ARRAY_WRITE(answerALL@@RANDOM_ID_3@@, 2, tmp32@@RANDOM_ID_5@@)
JR_GTEQ8(tmp@@RANDOM_ID_4@@, 0, m@@RANDOM_ID_8@@)
AND8(tmp@@RANDOM_ID_4@@, 127, tmp@@RANDOM_ID_4@@)
MOVE8_32(tmp@@RANDOM_ID_4@@, tmp32@@RANDOM_ID_5@@)
ADD32(tmp32@@RANDOM_ID_5@@, 128, tmp32@@RANDOM_ID_5@@)
ARRAY_WRITE(answerALL@@RANDOM_ID_3@@, 2, tmp32@@RANDOM_ID_5@@)

m@@RANDOM_ID_8@@:
ARRAY_READ(answer@@RANDOM_ID_2@@, 3, tmp@@RANDOM_ID_4@@)
MOVE8_32(tmp@@RANDOM_ID_4@@, tmp32@@RANDOM_ID_5@@)
ARRAY_WRITE(answerALL@@RANDOM_ID_3@@, 1, tmp32@@RANDOM_ID_5@@)
JR_GTEQ8(tmp@@RANDOM_ID_4@@, 0, m@@RANDOM_ID_9@@)
AND8(tmp@@RANDOM_ID_4@@, 127, tmp@@RANDOM_ID_4@@)
MOVE8_32(tmp@@RANDOM_ID_4@@, tmp32@@RANDOM_ID_5@@)
ADD32(tmp32@@RANDOM_ID_5@@, 128, tmp32@@RANDOM_ID_5@@)
ARRAY_WRITE(answerALL@@RANDOM_ID_3@@, 1, tmp32@@RANDOM_ID_5@@)

m@@RANDOM_ID_9@@:
ARRAY_READ(answer@@RANDOM_ID_2@@, 4, tmp@@RANDOM_ID_4@@)
MOVE8_32(tmp@@RANDOM_ID_4@@, tmp32@@RANDOM_ID_5@@)
ARRAY_WRITE(answerALL@@RANDOM_ID_3@@, 0, tmp32@@RANDOM_ID_5@@)
JR_GTEQ8(tmp@@RANDOM_ID_4@@, 0, m@@RANDOM_ID_10@@)
AND8(tmp@@RANDOM_ID_4@@, 127, tmp@@RANDOM_ID_4@@)
MOVE8_32(tmp@@RANDOM_ID_4@@, tmp32@@RANDOM_ID_5@@)
ADD32(tmp32@@RANDOM_ID_5@@, 128, tmp32@@RANDOM_ID_5@@)
ARRAY_WRITE(answerALL@@RANDOM_ID_3@@, 0, tmp32@@RANDOM_ID_5@@)

m@@RANDOM_ID_10@@:
ARRAY(DELETE, answer@@RANDOM_ID_2@@)
ARRAY(DELETE, ar@@RANDOM_ID_1@@)

DATA32 sizeRes@@RANDOM_ID_11@@
ARRAY(SIZE, @@RESULT@@, sizeRes@@RANDOM_ID_11@@)
JR_GT32(sizeRes@@RANDOM_ID_11@@, 255, m@@RANDOM_ID_12@@)
ARRAY(DELETE, @@RESULT@@)
m@@RANDOM_ID_12@@:

MOVE32_32(answerALL@@RANDOM_ID_3@@, @@RESULT@@)