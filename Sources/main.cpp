#include <3ds.h>
#include "csvc.h"
#include "cheats.hpp"

#include <vector>

namespace CTRPluginFramework
{
    // これは、amiiboをスキャンするときにタッチスクリーンを無効にするNFCにパッチを適用し、ctrpfが使用されないようにします。
    static void    ToggleTouchscreenForceOn(void)
    {
        static u32 original = 0;
        static u32 *patchAddress = nullptr;

        if (patchAddress && original)
        {
            *patchAddress = original;
            return;
        }

        static const std::vector<u32> pattern =
        {
            0xE59F10C0, 0xE5840004, 0xE5841000, 0xE5DD0000,
            0xE5C40008, 0xE28DD03C, 0xE8BD80F0, 0xE5D51001,
            0xE1D400D4, 0xE3510003, 0x159F0034, 0x1A000003
        };

        Result  res;
        Handle  processHandle;
        s64     textTotalSize = 0;
        s64     startAddress = 0;
        u32 *   found;

        if (R_FAILED(svcOpenProcess(&processHandle, 16)))
            return;

        svcGetProcessInfo(&textTotalSize, processHandle, 0x10002);
        svcGetProcessInfo(&startAddress, processHandle, 0x10005);
        if(R_FAILED(svcMapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, processHandle, (u32)startAddress, textTotalSize)))
            goto exit;

        found = (u32 *)Utils::Search<u32>(0x14000000, (u32)textTotalSize, pattern);

        if (found != nullptr)
        {
            original = found[13];
            patchAddress = (u32 *)PA_FROM_VA((found + 13));
            found[13] = 0xE1A00000;
        }

        svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, textTotalSize);
	exit:
        svcCloseHandle(processHandle);
    }

    // この関数は、メインの前とゲームの開始前に呼び出されます
    // コード編集を安全に行うのに便利です
    void    PatchProcess(FwkSettings &settings)
    {
        ToggleTouchscreenForceOn();
    }

    // この関数は、プロセスが終了したときに呼び出されます
    // 設定の保存、パッチの取り消し、またはクリーンアップに役立ちます
    void    OnProcessExit(void)
    {
        ToggleTouchscreenForceOn();
    }

	// メニュー
	// new MenuEntry("チート名", cheats.cppのチート名), 通常
	// new MenuEntry("チート名", nullptr, cheats.cppのチート名),　歯車
    void    InitMenu(PluginMenu &menu)
    {
        menu += new MenuFolder(Color::Red<< "フォルダ名", "",
		{
			
		});
    }

    int     main(void)
    {
		//Pluginのバージョンと説明
        PluginMenu *menu = new PluginMenu("名前", 1, 0, 0,
                                            "説明");

        // メニューをフレームイベントと同期します
        menu->SynchronizeWithFrame(true);

        // メニューエントリとフォルダを初期化する
        InitMenu(*menu);

        // 起動メニューとメインループ
        menu->Run();

        delete menu;

        // Pluginを終了します
        return (0);
    }
}
