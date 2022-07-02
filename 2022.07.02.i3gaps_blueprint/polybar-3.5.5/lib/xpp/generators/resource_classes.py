_resource_classes = \
    {
    ### XPROTO ###
      "WINDOW"
    , "PIXMAP"
    , "CURSOR"
    , "FONT"
    , "GCONTEXT"
    , "COLORMAP"
    , "ATOM"
    , "DRAWABLE"
    , "FONTABLE"
    ### XPROTO ###

    ### DAMAGE ###
    , "DAMAGE"
    ### DAMAGE ###

    ### GLX, RECORD, XVMC ###
    , "CONTEXT"
    ### GLX, RECORD, XVMC ###

    ### GLX ###
    # , "PIXMAP" # already in XPROTO
    # , "CONTEXT"
    , "PBUFFER"
    # , "WINDOW" # already in XPROTO
    , "FBCONFIG"
    ### GLX ###

    ### PRESENT ###
    , "EVENT"
    ### PRESENT ###

    ### RANDR ###
    , "MODE"
    , "CRTC"
    , "OUTPUT"
    , "PROVIDER"
    ### RANDR ###

    ### RECORD ###
    # , "CONTEXT"
    ### RECORD ###

    ### RENDER ###
    , "GLYPHSET"
    , "PICTURE"
    , "PICTFORMAT"
    ### RENDER ###

    ### SHM ###
    , "SEG"
    ### SHM ###

    ### SYNC ###
    , "ALARM"
    , "COUNTER"
    , "FENCE"
    ### SYNC ###

    ### XFIXES ###
    , "REGION"
    , "BARRIER"
    ### XFIXES ###

    ### XPRINT ###
    , "PCONTEXT"
    ### XPRINT ###

    ### XVMC ###
    # , "CONTEXT"
    , "SURFACE"
    , "SUBPICTURE"
    ### XVMC ###

    ### XV ###
    , "PORT"
    , "ENCODING"
    ### XV ###
    }
