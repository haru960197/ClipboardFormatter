#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <regex> // 正規表現ライブラリを追加

// クリップボードの内容を取得
std::string getClipboardText() {
    if (!OpenClipboard(nullptr)) {
        std::cerr << "Failed to open clipboard" << std::endl;
        return "";
    }

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) {
        std::cerr << "Failed to get clipboard data" << std::endl;
        CloseClipboard();
        return "";
    }

    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        std::cerr << "Failed to lock clipboard data" << std::endl;
        CloseClipboard();
        return "";
    }

    std::string text(pszText);

    GlobalUnlock(hData);
    CloseClipboard();

    return text;
}

// 文字列をクリップボードにセット
void setClipboardText(const std::string& text) {
    if (!OpenClipboard(nullptr)) {
        std::cerr << "Failed to open clipboard" << std::endl;
        return;
    }

    EmptyClipboard();

    HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, text.size() + 1);
    if (hGlob == nullptr) {
        std::cerr << "Failed to allocate memory" << std::endl;
        CloseClipboard();
        return;
    }

    memcpy(hGlob, text.c_str(), text.size() + 1);

    SetClipboardData(CF_TEXT, hGlob);
    CloseClipboard();
}

/**
 * 指定されたパターンの連続する数字を除去する
 */
std::string removeDigitPatterns(const std::string& text) {
    // 2つのパターンを正規表現で定義
    //   パターン1: (\d)\1{3,}
    //     (\d)    : 任意の数字1文字をキャプチャ (グループ1)
    //     \1{3,}  : グループ1と同じ文字が3回以上続く (合計4回以上)
    //     (例: "0000", "77777")
    //   
    //   パターン2: (\d)(?!\2)(\d)\2\3(\2\3)*\2?
    //     (\d)      : 任意の数字1文字をキャプチャ (グループ2 'a')
    //     (?!\2)(\d): グループ2と異なる数字1文字をキャプチャ (グループ3 'b')
    //     \2\3      : "ab" のパターン (例: "01")
    //     (\2\3)* : "abab", "ababab"... (0回以上の繰り返し)
    //     \2?       : 最後に 'a' が来ても良い (例: "010")
    //     -> これらを組み合わせることで "abab" (4桁) 以上のパターンに一致
    //     (例: "0101", "01010", "898989")
    //
    // R"(...)" はC++11のRaw文字列リテラルで、バックスラッシュをエスケープせずに書けます
    const std::regex pattern(R"((\d)\1{3,}|(\d)(?!\2)(\d)\2\3(\2\3)*\2?)");

    // 一致した部分を空文字列 "" に置換
    return std::regex_replace(text, pattern, "");
}


int main() {
    // 1. クリップボードから文字列を取得
    std::string clipboardText = getClipboardText();
    if (clipboardText.empty()) {
        std::cerr << "Clipboard is empty or error occurred" << std::endl;
        return 1;
    }

    // 2. 連続する数字パターンを除去
    std::string filteredText = removeDigitPatterns(clipboardText);

    // 3. 置換後の文字列をクリップボードに上書き
    setClipboardText(filteredText);

    std::cout << "Clipboard text modified successfully!" << std::endl;

    return 0;
}

/**
 * 以下でコンパイル
 * * $ g++ -o clipboard_replace3 clipboard_gemini_number_remover.cpp -static-libgcc -static-libstdc++ -mwindows
 */
