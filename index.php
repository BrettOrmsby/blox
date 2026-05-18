<?php

$code = <<<"EOT"
-->+++>+>+>+>+++++>++>++>->+++>++>+>>>>>>>>>>>>>>>>->++++>>>>->+++>+++>+++>+++>+
++>+++>+>+>>>->->>++++>+>>>>->>++++>+>+>>->->++>++>++>++++>+>++>->++>++++>+>+>++
>++>->->++>++>++++>+>+>>>>>->>->>++++>++>++>++++>>>>>->>>>>+++>->++++>->->->+++>
>>+>+>+++>+>++++>>+++>->>>>>->>>++++>++>++>+>+++>->++++>>->->+++>+>+++>+>++++>>>
+++>->++++>>->->++>++++>++>++++>>++[-[->>+[>]++[<]<]>>+[>]<--[++>++++>]+[<]<<++]
>>>[>]++++>++++[--[+>+>++++<<[-->>--<<[->-<[--->>+<<[+>+++<[+>>++<<]]]]]]>+++[>+
++++++++++++++<-]>--.<<<]
EOT;

$c = 0;
$arr = array_fill(0, 3000, 0);
$p = 0;

while ($c < strlen($code)) {
    $char = $code[$c];

    match ($char) {
        ">" => $p++,
        "<" => $p--,
        "+" => $arr[$p]++,
        "-" => $arr[$p]--,
        "." => output(),
        "," => input(),
        "[" => jumpForward(),
        "]" => jumpBack(),
        default => null,
    };
    // echo "I read" . $char . PHP_EOL;
    $c++;
}

function output()
{
    global $arr;
    global $p;
    // echo "I output: " . $arr[$p] . PHP_EOL;
    echo chr($arr[$p]);
}
function input()
{
    global $arr;
    global $p;

    $input = fgetc(STDIN);
    // echo "I read" . $input;
    $arr[$p] = ord($input[0]);
}
function jumpForward()
{
    global $arr;
    global $p;
    global $c;
    global $code;

    if ($arr[$p] != 0) {
        return;
    }

    $pairs = 1;

    while ($c < strlen($code)-1 && $pairs > 0) {
        match ($code[++$c]) {
            "[" => $pairs++,
            "]" => $pairs--,
            default => null,
        };
    }
}
function jumpBack()
{
    global $arr;
    global $p;
    global $c;
    global $code;

    if ($arr[$p] == 0) {
        return;
    }

    $pairs = 1;
    while ($c >= 1 && $pairs > 0) {
        match ($code[--$c]) {
            "[" => $pairs--,
            "]" => $pairs++,
            default => null,
        };
    }
}
