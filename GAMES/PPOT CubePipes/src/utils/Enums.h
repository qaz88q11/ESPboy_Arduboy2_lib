#pragma once

enum PuzzleStatus : uint8_t { 
    Locked,
    Complete,
    InProgress,
};

enum PuzzleSize : uint8_t { 
    Small,
    Large,
};

enum class GameOver : uint8_t { 
    No,
    LevelOver, 
    GameOver,
};

enum Direction : uint8_t { 
    None,
    Left,
    Right,
};

enum class GameState : uint8_t {

    SplashScreen_Start,
        SplashScreen_00 = SplashScreen_Start,
        SplashScreen_01,
        SplashScreen_02,
        SplashScreen_03,
    SplashScreen_End,

    Title_Init,
    Title_Start,
        Title_Main = Title_Start,
        Title_Size,
        Title_Select,
        Title_Clear_Progress,
    Title_End,

    Play_Init,
    Play_Start,
        Play = Play_Start,
        Play_FadeIn,
    Play_End,
    
};
