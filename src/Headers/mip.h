/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 *                   Oasis olc header module                                *
 ****************************************************************************/

#define MIP_MEDIA_URL           "http://www.your-site.com/mip/"

DECLARE_DO_FUN( do_mip_start );

    bool    mip_enabled     args( ( CHAR_DATA *ch ) );
    void    init_mip        args( ( CHAR_DATA *ch ) );
    void    send_mip        args( ( char *argument, CHAR_DATA *ch ) );
    void    send_mip_music  args( ( CHAR_DATA *ch, char *argument, int iterations ) );
    void    send_mip_sound  args( ( CHAR_DATA *ch, char *filename ) );
    void    send_mip_image  args( ( CHAR_DATA *ch, char *filename, char *label ) );
    void    send_mip_reboot args( ( CHAR_DATA *ch, char *argument ) );
    void    send_mip_uptime args( ( CHAR_DATA *ch, char *argument ) );
    void    send_mip_avi    args( ( CHAR_DATA *ch, char *filename, char *label, int height, int width, bool fRepeat ) );
    void    send_mip_special  args( ( CHAR_DATA *ch, char *argument ) );
    void    send_mip_special2 args( ( CHAR_DATA *ch, char *argument ) );
    void    send_mip_tell     args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *argument, bool fReceiving ) );
    void    send_mip_imc_tell args( ( char *from, CHAR_DATA *victim, char *argument) );
    void    send_mip_room   args( ( CHAR_DATA *ch, char *argument ) );
    void    send_mip_mudlag args( ( CHAR_DATA *ch, char *argument ) );
    void    send_mip_edit   args( ( CHAR_DATA *ch, char *filename ) );
    void    send_mip_mask   args( ( CHAR_DATA *ch, char *argument, char *type ) );
    void    send_mip_caption  args( ( CHAR_DATA *ch, char *argument ) );   
    void    send_mip_begin_file args( ( CHAR_DATA *ch, int lines, char *filename ) );
    void    send_mip_cont_file  args( ( CHAR_DATA *ch, char *line ) );
    void    send_mip_end_file   args( ( CHAR_DATA *ch ) );
    void    send_mip_channel  args( ( CHAR_DATA *ch, const char *cmd, char *channel, char *source, char *argument ) );
    void    send_mip_exits  args( ( CHAR_DATA *ch ) );
    void    send_mip_points args( ( CHAR_DATA *ch ) );
    void    send_mip_attacker args( ( CHAR_DATA *ch ) );

    #define CL_DELIM                "~"  
    #define CL_SEND_SOUND           "AAA"
    #define CL_SEND_IMAGE           "AAB"
    #define CL_SEND_REBOOT          "AAC"
    #define CL_SEND_MUSIC           "AAD"
    #define CL_SEND_UPTIME          "AAF"
    #define CL_SEND_AVI             "AAG"
    #define CL_DOWNLOAD_MEDIA       "AAH"
    #define CL_SEND_SPECIAL         "BAA"
    #define CL_SEND_SPECIAL2        "BAC"
    #define CL_SEND_TELL            "BAB"
    #define CL_SEND_ROOM            "BAD"
    #define CL_SEND_MUDLAG          "BAE"
    #define CL_SEND_EDIT            "BAF"
    #define CL_GP1_MASK             "BBA"
    #define CL_GP2_MASK             "BBB"
    #define CL_HP_MASK              "BBC"
    #define CL_SP_MASK              "BBD"
    #define CL_SEND_CAPTION         "CAP"
    #define CL_SEND_BEGIN_FILE      "CDF"
    #define CL_SEND_CONT_FILE       "CCF"
    #define CL_SEND_END_FILE        "CEF"
    #define CL_SEND_CHAT            "CAA"
    #define CL_SEND_ROOMCODE        "DDD"
    #define CL_SEND_COMPOSITE       "FFF"
    #define CL_SEND_HP              "A"
    #define CL_SEND_MAXHP           "B"  
    #define CL_SEND_SP              "C"
    #define CL_SEND_MAXSP           "D"
    #define CL_SEND_GP1             "E"  
    #define CL_SEND_MAXGP1          "F"
    #define CL_SEND_GP2             "G"  
    #define CL_SEND_MAXGP2          "H"  
    #define CL_SEND_GLINE1          "I"  
    #define CL_SEND_GLINE2          "J"  
    #define CL_SEND_ATTACKER        "K"  
    #define CL_SEND_ATTCOND         "L"
    #define CL_SEND_ATTIMG          "M"

