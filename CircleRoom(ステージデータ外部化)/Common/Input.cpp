#include "Input.h"
#include <DxLib.h>
#include <cassert>
#include <string>
#include <algorithm>

using namespace std;

namespace
{
    constexpr float kVersion = 1.3f;

    struct KeyConfHeader
    {
        const char* const id = "kyc"; // 最後に'\0'入ってるので4バイト
        float version = kVersion;
        size_t dataCount = 0;
        // 空白の4バイト(パディング)
    };
}

const InputTable_t Input::GetCommandTable() const
{
    InputTable_t ret = m_commandTable;
    for (const auto& ex : m_exclusiveKeyConfigCommands)
    {
        ret.erase(ex);
    }

    return ret;
}

Input::Input() :
    m_isAnyPush(false),
    m_isLastAnyPush(false),
    m_lastType(InputType::pad)
{
    // メニュー関連
    // 見せるもの
    m_commandTable["OK"] = { {InputType::keybd, { KEY_INPUT_X } } ,
                             {InputType::pad,   { PAD_INPUT_A } } };
    m_commandTable["cancel"] = { {InputType::keybd, { KEY_INPUT_C } } ,
                             {InputType::pad,   { PAD_INPUT_B } } };     // 
    m_commandTable["pause"] = { {InputType::keybd,  { KEY_INPUT_ESCAPE } },
                                {InputType::pad,    { PAD_INPUT_R } } }; // スタートボタン
    // 見せないもの
    m_commandTable["optionLeft"] = { {InputType::keybd, { KEY_INPUT_Q } },
                                     {InputType::pad,   { PAD_INPUT_5 } } };
    m_commandTable["optionRight"] = { {InputType::keybd, { KEY_INPUT_E } },
                                     {InputType::pad,   { PAD_INPUT_6 } } };

    // ゲーム中関連
    // 見せるもの
    m_commandTable["dash"] = { {InputType::keybd,  { KEY_INPUT_SPACE } },
                                {InputType::pad,    { PAD_INPUT_A } } }; // キーコンフィグ

    // 見せないもの
    m_commandTable["up"] = { {InputType::keybd,  { KEY_INPUT_UP, KEY_INPUT_W } },
                             {InputType::pad,    { PAD_INPUT_UP } } };
    m_commandTable["down"] = { {InputType::keybd,  { KEY_INPUT_DOWN , KEY_INPUT_S }},
                             {InputType::pad,    { PAD_INPUT_DOWN } } };
    m_commandTable["left"] = { {InputType::keybd,  { KEY_INPUT_LEFT , KEY_INPUT_A }},
                             {InputType::pad,    { PAD_INPUT_LEFT  } } };
    m_commandTable["right"] = { {InputType::keybd,  { KEY_INPUT_RIGHT , KEY_INPUT_D }},
                             {InputType::pad,    { PAD_INPUT_RIGHT } } };

    m_exclusiveKeyConfigCommands = {"optionLeft", "optionRight", "up", "down", "left", "right"};

    Load(L"Data/Bin/key.cnf");

    auto& pad = m_corrTable[InputType::pad];
    pad[PAD_INPUT_A] = L"ＡBottan";
    pad[PAD_INPUT_B] = L"ＢBottan";
    pad[PAD_INPUT_C] = L"ＸBottan";
    pad[PAD_INPUT_X] = L"ＹBottan";
    pad[PAD_INPUT_5] = L"ＬBottan";
    pad[PAD_INPUT_6] = L"ＲBottan";
    pad[PAD_INPUT_R] = L"ＳＴＡＲＴBottan";
    pad[PAD_INPUT_L] = L"ＭBottan";

    auto& keybd = m_corrTable[InputType::keybd];
    keybd[KEY_INPUT_A] = L"Ａキー";
    keybd[KEY_INPUT_B] = L"Ｂキー";
    keybd[KEY_INPUT_C] = L"Ｃキー";
    keybd[KEY_INPUT_D] = L"Ｄキー";
    keybd[KEY_INPUT_E] = L"Ｅキー";
    keybd[KEY_INPUT_F] = L"Ｆキー";
    keybd[KEY_INPUT_G] = L"Ｇキー";
    keybd[KEY_INPUT_H] = L"Ｈキー";
    keybd[KEY_INPUT_I] = L"Ｉキー";
    keybd[KEY_INPUT_J] = L"Ｊキー";
    keybd[KEY_INPUT_K] = L"Ｋキー";
    keybd[KEY_INPUT_L] = L"Ｌキー";
    keybd[KEY_INPUT_M] = L"Ｍキー";
    keybd[KEY_INPUT_N] = L"Ｎキー";
    keybd[KEY_INPUT_O] = L"Ｏキー";
    keybd[KEY_INPUT_P] = L"Ｐキー";
    keybd[KEY_INPUT_Q] = L"Ｑキー";
    keybd[KEY_INPUT_R] = L"Ｒキー";
    keybd[KEY_INPUT_S] = L"Ｓキー";
    keybd[KEY_INPUT_T] = L"Ｔキー";
    keybd[KEY_INPUT_U] = L"Ｕキー";
    keybd[KEY_INPUT_V] = L"Ｖキー";
    keybd[KEY_INPUT_W] = L"Ｗキー";
    keybd[KEY_INPUT_X] = L"Ｘキー";
    keybd[KEY_INPUT_Y] = L"Ｙキー";
    keybd[KEY_INPUT_Z] = L"Ｚキー";
    keybd[KEY_INPUT_BACK] = L"BSキー";
    keybd[KEY_INPUT_TAB] = L"Tabキー";
    keybd[KEY_INPUT_RETURN] = L"Enterキー";
    keybd[KEY_INPUT_LSHIFT] = L"左Shiftキー";
    keybd[KEY_INPUT_RSHIFT] = L"右Shiftキー";
    keybd[KEY_INPUT_LCONTROL] = L"左Ctrlキー";
    keybd[KEY_INPUT_RCONTROL] = L"右Ctrlキー";
    keybd[KEY_INPUT_ESCAPE] = L"Escキー";
    keybd[KEY_INPUT_SPACE] = L"スペースキー";
}

void Input::Update()
{
    m_lastInputDate = m_inputDate;  // 直前入力をコピーしておく
    m_isLastAnyPush = m_isAnyPush;

    /* ハードウェア入力チェック */
    // キーボード用
    char keystate[256]; 
    GetHitKeyStateAll(keystate);   // 現在のキーボード入力を取得
    // パッド情報の取得
    int padstate = GetJoypadInputState(DX_INPUT_PAD1);  
    // スティック情報の取得
    int stickX, stickY;
    GetJoypadAnalogInput(&stickX, &stickY, DX_INPUT_PAD1);
    m_inputStickDate.x = static_cast<float>(stickX);
    m_inputStickDate.y = static_cast<float>(stickY);

    if (padstate || CheckHitKeyAll())
    {
        m_isAnyPush = true;
    }
    else
    {
        m_isAnyPush = false;
    }

    /*情報更新*/
    // 登録された情報とハードの情報を照らし合わせながら、
    // m_inputDataの内容を更新していく
    for (const auto& cmd : m_commandTable)
    {
        // コマンドの名前から入力データを作る
        auto& input = m_inputDate[cmd.first];

        for (const auto& hardIO : cmd.second)
        {
            input = false;
            // キーボードのチェック
            if (hardIO.first == InputType::keybd)
            {
                for (const auto& code : hardIO.second)
                {
                    if (keystate[code])
                    {
                        input = true;
                        m_lastType = hardIO.first;
                        break;
                    }
                }
                if (input) break;
            }
            else if (hardIO.first == InputType::pad)
            {
                for (const auto& code : hardIO.second)
                {
                    if (padstate & code)
                    {
                        input = true;
                        m_lastType = hardIO.first;
                        break;
                    }
                }
                if (input) break;
            }
        }
    }
}

bool Input::IsTriggered(const char* command) const
{
    auto it = m_inputDate.find(command);
    if (it == m_inputDate.end())
    {
        assert(false);
        return false;
    }

    return m_inputDate.at(command) && !m_lastInputDate.at(command);

}

bool Input::IsAnyTriggerd()
{
    return m_isAnyPush && !m_isLastAnyPush;
}

bool Input::IsPress(const char* command) const
{
    auto it = m_inputDate.find(command);
    if (it == m_inputDate.end())
    {
        assert(false);
        return false;
    }

    return m_inputDate.at(command);
}

bool Input::IsAnyPress() const
{
    return m_isAnyPush;
}

bool Input::IsNotPress(const char* command) const
{
    auto it = m_inputDate.find(command);
    if (it == m_inputDate.end())
    {
        assert(false);
        return false;
    }

    return !m_inputDate.at(command);
}

bool Input::IsReleased(const char* command) const
{
    auto it = m_inputDate.find(command);
    if (it == m_inputDate.end())
    {
        assert(false);
        return false;
    }

    return !m_inputDate.at(command) && m_lastInputDate.at(command);
}

bool Input::IsReepat(const char* command, int& frame, int frameInterval) const
{
    // 押されていない時はフレームの初期化のみ行う
    if (IsNotPress(command))
    {
        frame = 0;
        return false;
    }
    // 押された瞬間はture
    if (IsTriggered(command))
    {
        frame = 0;
        return true;
    }

    frame++;
    // 間隔時間を超えたらtrueとする
    if (frame > frameInterval)
    {
        frame = 0;
        return true;
    }

    return false;
}

std::wstring Input::GetHardDataName(const std::string cmd, InputType type) const
{
    return  m_corrTable.at(type).at(m_commandTable.at(cmd).at(type)[0]);
}

void Input::Save(const std::string& path)
{
    FILE* fp = nullptr; // ファイルポインタ
    auto err = fopen_s(&fp, path.c_str(), "wb");
    if (err != 0)
    {
        // 読み込みに失敗したため終了
        assert(false);
        return;
    }
    // ヘッダの書き込み
    KeyConfHeader header;
    header.dataCount = m_commandTable.size();
    fwrite(&header, sizeof(header), 1, fp);

    // データ本体を書き込んでいく
    for (const auto& cmd : m_commandTable)
    {
        const auto& commandStr = cmd.first; // コマンド文字列
        uint8_t m_size = static_cast<uint8_t>(commandStr.size());   // コマンド文字列のサイズを取得
        fwrite(&m_size, sizeof(m_size), 1, fp); // コマンド文字列のバイト数を書き込む
        fwrite(commandStr.data(), commandStr.size(), 1, fp);    // 文字列の書き込み

        const auto& InputData = cmd.second; // 対応する入力
        uint8_t inputTypeSize = static_cast<uint8_t>(InputData.size());   // 対応する入力の数(基本的に2, キーボードとパッド)
        fwrite(&inputTypeSize, sizeof(inputTypeSize), 1, fp);   // 2を書き込む
        for (const auto& input : InputData)
        {
            const auto& type = input.first; // キーボードかパッドか
            fwrite(&type, sizeof(type), 1, fp);

            // サイズの読み込み
            uint8_t size = static_cast<uint8_t>(input.second.size());
            fwrite(&size, sizeof(size), 1, fp);

            for (const auto& state : input.second)
            {
                // 実際の入力ID
                fwrite(&state, sizeof(state), 1, fp);
            }
        }
    }

    fclose(fp);
}

void Input::Load(const std::wstring& path)
{
    auto handle = FileRead_open(path.c_str());
    // エラー(ファイルがない)場合は処理しない
    if (handle == 0)
    {
        return;
    }
    // ヘッダの読み込み
    KeyConfHeader header;
    FileRead_read(&header, sizeof(header), handle);
    // バージョンが違う場合は読み込みしない
    if (header.version != kVersion)
    {
        FileRead_close(handle);
        return;
    }

    // データの読み込み
    for (int i = 0; i < header.dataCount; i++)
    {
        uint8_t cmdStrSize = 0; // コマンド文字列のサイズ
        FileRead_read(&cmdStrSize, sizeof(cmdStrSize), handle); // コマンド文字列サイズ読み込み
        std::string cmdStr; // コマンド文字列
        cmdStr.resize(cmdStrSize);  // コマンド文字列サイズでリサイズ
        FileRead_read(cmdStr.data(), static_cast<int>(cmdStr.size() * sizeof(char)), handle);   // コマンド文字列の読み込み

        auto& command = m_commandTable[cmdStr]; // コマンドテーブルから対象のコマンドの参照を取得

        uint8_t inputTypeSize;  // 入力種別数を取得してくる(基本的には2が入ってる)
        FileRead_read(&inputTypeSize, sizeof(inputTypeSize), handle);   // 種別数の取得
        for (int j = 0; j < inputTypeSize; j++) // 取得した種別数だけループする
        {
            InputType type;
            uint8_t size;
            std::vector<int> state;

            FileRead_read(&type, sizeof(type), handle); // 種別を読みこむ

            // サイズ読み込み
            FileRead_read(&size, sizeof(size), handle);
            state.resize(size);

            for (int i = 0; i < size; i++)
            {
                // 実際の入力ステート情報を読み込む
                FileRead_read(&state[i], sizeof(state[i]), handle);
            }

            command[type] = state;  // コマンドに反映させる
        }
    }

    FileRead_close(handle);
}
