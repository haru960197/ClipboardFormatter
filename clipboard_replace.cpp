#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>

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

// 改行文字を半角スペースに置換
std::string replaceNewlinesWithSpaces(const std::string& text) {
    std::string result = text;
    for (char& c : result) {
        if (c == '\n' || c == '\r') {
            c = ' ';
        }
    }
    return result;
}

// 連続する半角スペースを1つに置換
std::string reduceMultipleSpaces(const std::string& text) {
    std::stringstream ss(text);
    std::string word, result;
    bool firstWord = true;

    // 文字列をスペースで分割し、単語を1つずつ処理
    while (ss >> word) {
        if (!firstWord) {
            result += " "; // 単語間にスペースを挿入
        }
        result += word;
        firstWord = false;
    }

    return result;
}

int main() {
    // 1. クリップボードから文字列を取得
    std::string clipboardText = getClipboardText();
    if (clipboardText.empty()) {
        std::cerr << "Clipboard is empty or error occurred" << std::endl;
        return 1;
    }

    // 2. 改行文字を半角スペースに置換
    std::string replacedText = replaceNewlinesWithSpaces(clipboardText);

    // 3. 連続する半角スペースを1つにまとめる
    std::string finalText = reduceMultipleSpaces(replacedText);

    // 4. 置換後の文字列をクリップボードに上書き
    setClipboardText(finalText);

    std::cout << "Clipboard text modified successfully!" << std::endl;

    return 0;
}

/**
 * 以下でコンパイル
 * 
 * $ g++ -o clipboard_replace clipboard_replace.cpp -static-libgcc -static-libstdc++ -mwindows
 */