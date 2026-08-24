*---------------------------------------------------------------------*
* C bridge for RACROUTE REQUEST=AUTH.                                 *
*---------------------------------------------------------------------*
         SYSSTATE AMODE64=YES,ARCHLVL=OSREL,OSREL=SYSSTATE

*---------------------------------------------------------------------*
* Register equates.                                                   *
*---------------------------------------------------------------------*
R0       EQU   0
R1       EQU   1
R2       EQU   2
R3       EQU   3
R4       EQU   4
R5       EQU   5
R6       EQU   6
R7       EQU   7
R8       EQU   8
R9       EQU   9
R10      EQU   10
R11      EQU   11
R12      EQU   12
R13      EQU   13
R14      EQU   14
R15      EQU   15

RACFAUTH TITLE 'RACROUTE AUTHORIZATION'

RACFAUTH CELQPRLG DSASIZE=DSASIZ,                                      X
               BASEREG=R12,                                            X
               PARMWRDS=3,                                             X
               PARMREG=R11,                                            X
               ENTNAME=callRauth,                                      X
               PSECT=RACFAUT1

callRauth ALIAS C'sear_racroute_auth_asm'
         USING AUTOSTG,R4
         USING MYPARMS,R11

         LGR   R10,R5
         USING RACFAUT1,R10
         XC    RACF_RC,RACF_RC
         XC    RACF_RSN,RACF_RSN
         XC    STOR31PTR,STOR31PTR

         XGR   R1,R1
         AGHI  R1,STOR31L
         STG   R1,PARM1

         LMG   R5,R6,MALL31FD
         BASR  R7,R6
         NOPR  0

CEEWSA   LOCTR
C_WSA64  CATTR DEFLOAD,RMODE(64),PART(RACFAUT1)
MALLOC31 ALIAS C'__malloc31'
MALLOC31 AMODE 64
MALLOC31 XATTR LINKAGE(XPLINK),SCOPE(IMPORT),REF(CODE)
MALL31FD DC    RD(MALLOC31)
         DC    VD(MALLOC31)
FREE     ALIAS C'free'
FREE     AMODE 64
FREE     XATTR LINKAGE(XPLINK),SCOPE(IMPORT),REF(CODE)
FREE_FD  DC    RD(FREE)
         DC    VD(FREE)
RACFAUTH LOCTR

         LGR   R5,R10
         LTR   R3,R3
         JZ    NOMALL31
         LGR   R8,R3
         STG   R8,STOR31PTR
         USING STOR31,R8

         XC    RACF_RC,RACF_RC
         XC    RACF_RSN,RACF_RSN

         LG    R9,REQPTR
         LTGR  R9,R9
         JZ    BADINPUT

         LA    R1,RACREQ_CLASS(,R9)
         L     R2,RACREQ_CLASSLEN(,R9)
         LTGR  R1,R1
         JZ    BADINPUT
         LTR   R2,R2
         BNP   BADINPUT
         CHI   R2,8
         JH    BADINPUT

         STC   R2,CLASSBUF31
         MVI   CLASSBUF31+1,C' '
         MVC   CLASSBUF31+2(7),CLASSBUF31+1
         LR    R3,R2
         BCTR  R3,0
         EX    R3,COPYCLASS

         LA    R1,RACREQ_ENTITY(,R9)
         L     R2,RACREQ_ENTITYLEN(,R9)
         LTGR  R1,R1
         JZ    BADINPUT
         LTR   R2,R2
         BNP   BADINPUT
         CHI   R2,246
         JH    BADINPUT

         STH   R2,ENTITYBUF31
         STH   R2,ENTITYBUF31+2
         MVI   ENTITYBUF31+4,C' '
         MVC   ENTITYBUF31+5(245),ENTITYBUF31+4
         LR    R3,R2
         BCTR  R3,0
         EX    R3,COPYENTITY

         LA    R1,RACREQ_AUTHID(,R9)
         L     R2,RACREQ_AUTHIDLEN(,R9)
         LTGR  R1,R1
         JZ    BADINPUT
         LTR   R2,R2
         BNP   BADINPUT
         CHI   R2,8
         JH    BADINPUT

         MVI   AUTHIDB31,C' '
         MVC   AUTHIDB31+1(7),AUTHIDB31
         LR    R3,R2
         BCTR  R3,0
         EX    R3,COPYAUTH

         L     R2,RACREQ_IDTYPE(,R9)
         CHI   R2,0
         JE    CHECKSTAT
         CHI   R2,1
         JE    CHECKSTAT
         J     BADINPUT

CHECKSTAT DS   0H
         L     R2,RACREQ_STATUS(,R9)
         CHI   R2,0
         JE    CHECKACCESS
         CHI   R2,1
         JE    AUTHACCESS
         J     BADINPUT

CHECKACCESS DS 0H
         L     R2,RACREQ_ACCESS(,R9)
         CHI   R2,X'02'
         JE    AUTHREAD
         CHI   R2,X'04'
         JE    AUTHUPDATE
         CHI   R2,X'08'
         JE    AUTHCONTROL
         CHI   R2,X'80'
         JE    AUTHALTER
         J     BADINPUT

AUTHREAD DS    0H
         L     R2,RACREQ_IDTYPE(,R9)
         CHI   R2,1
         JE    AUTHRDG
         MVC   RACFPL31(RACFPLRL),RACFPLR
         STG   R13,ORIGR13
         LA    R13,SAVEAREA31
         XC    SAVEAREA31,SAVEAREA31
         SAM31
         STMH  R2,R14,SAVEHIGH
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF31,CLASS=CLASSBUF31,WORKA=RACFWA31,    X
               MF=(E,RACFPL31)
         LMH   R2,R14,SAVEHIGH
         SAM64
         LG    R13,ORIGR13
         L     R2,RACFPL31
         ST    R2,RACF_RC
         L     R2,RACFPL31+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHRDG DS     0H
         MVC   RACFPL31(RACFPLRL),RACFPLR
         STG   R13,ORIGR13
         LA    R13,SAVEAREA31
         XC    SAVEAREA31,SAVEAREA31
         SAM31
         STMH  R2,R14,SAVEHIGH
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF31,CLASS=CLASSBUF31,WORKA=RACFWA31,    X
               MF=(E,RACFPL31)
         LMH   R2,R14,SAVEHIGH
         SAM64
         LG    R13,ORIGR13
         L     R2,RACFPL31
         ST    R2,RACF_RC
         L     R2,RACFPL31+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHUPDATE DS  0H
         L     R2,RACREQ_IDTYPE(,R9)
         CHI   R2,1
         JE    AUTHUPG
         MVC   RACFPL31(RACFPLUL),RACFPLU
         STG   R13,ORIGR13
         LA    R13,SAVEAREA31
         XC    SAVEAREA31,SAVEAREA31
         SAM31
         STMH  R2,R14,SAVEHIGH
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF31,CLASS=CLASSBUF31,WORKA=RACFWA31,    X
               MF=(E,RACFPL31)
         LMH   R2,R14,SAVEHIGH
         SAM64
         LG    R13,ORIGR13
         L     R2,RACFPL31
         ST    R2,RACF_RC
         L     R2,RACFPL31+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHUPG DS     0H
         MVC   RACFPL31(RACFPLUL),RACFPLU
         STG   R13,ORIGR13
         LA    R13,SAVEAREA31
         XC    SAVEAREA31,SAVEAREA31
         SAM31
         STMH  R2,R14,SAVEHIGH
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF31,CLASS=CLASSBUF31,WORKA=RACFWA31,    X
               MF=(E,RACFPL31)
         LMH   R2,R14,SAVEHIGH
         SAM64
         LG    R13,ORIGR13
         L     R2,RACFPL31
         ST    R2,RACF_RC
         L     R2,RACFPL31+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHCONTROL DS 0H
         L     R2,RACREQ_IDTYPE(,R9)
         CHI   R2,1
         JE    AUTHCTG
         MVC   RACFPL31(RACFPLCL),RACFPLC
         STG   R13,ORIGR13
         LA    R13,SAVEAREA31
         XC    SAVEAREA31,SAVEAREA31
         SAM31
         STMH  R2,R14,SAVEHIGH
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF31,CLASS=CLASSBUF31,WORKA=RACFWA31,    X
               MF=(E,RACFPL31)
         LMH   R2,R14,SAVEHIGH
         SAM64
         LG    R13,ORIGR13
         L     R2,RACFPL31
         ST    R2,RACF_RC
         L     R2,RACFPL31+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHCTG DS     0H
         MVC   RACFPL31(RACFPLCL),RACFPLC
         STG   R13,ORIGR13
         LA    R13,SAVEAREA31
         XC    SAVEAREA31,SAVEAREA31
         SAM31
         STMH  R2,R14,SAVEHIGH
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF31,CLASS=CLASSBUF31,WORKA=RACFWA31,    X
               MF=(E,RACFPL31)
         LMH   R2,R14,SAVEHIGH
         SAM64
         LG    R13,ORIGR13
         L     R2,RACFPL31
         ST    R2,RACF_RC
         L     R2,RACFPL31+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHALTER DS   0H
         L     R2,RACREQ_IDTYPE(,R9)
         CHI   R2,1
         JE    AUTHALG
         MVC   RACFPL31(RACFPLAL),RACFPLA
         STG   R13,ORIGR13
         LA    R13,SAVEAREA31
         XC    SAVEAREA31,SAVEAREA31
         SAM31
         STMH  R2,R14,SAVEHIGH
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF31,CLASS=CLASSBUF31,WORKA=RACFWA31,    X
               MF=(E,RACFPL31)
         LMH   R2,R14,SAVEHIGH
         SAM64
         LG    R13,ORIGR13
         L     R2,RACFPL31
         ST    R2,RACF_RC
         L     R2,RACFPL31+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHALG DS     0H
         MVC   RACFPL31(RACFPLAL),RACFPLA
         STG   R13,ORIGR13
         LA    R13,SAVEAREA31
         XC    SAVEAREA31,SAVEAREA31
         SAM31
         STMH  R2,R14,SAVEHIGH
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF31,CLASS=CLASSBUF31,WORKA=RACFWA31,    X
               MF=(E,RACFPL31)
         LMH   R2,R14,SAVEHIGH
         SAM64
         LG    R13,ORIGR13
         L     R2,RACFPL31
         ST    R2,RACF_RC
         L     R2,RACFPL31+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHACCESS DS  0H
         L     R2,RACREQ_IDTYPE(,R9)
         CHI   R2,1
         JE    AUTHACG
         MVC   RACFPL31(RACFPLSL),RACFPLS
         STG   R13,ORIGR13
         LA    R13,SAVEAREA31
         XC    SAVEAREA31,SAVEAREA31
         SAM31
         STMH  R2,R14,SAVEHIGH
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF31,CLASS=CLASSBUF31,WORKA=RACFWA31,    X
               MF=(E,RACFPL31)
         LMH   R2,R14,SAVEHIGH
         SAM64
         LG    R13,ORIGR13
         L     R2,RACFPL31
         ST    R2,RACF_RC
         L     R2,RACFPL31+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHACG DS     0H
         MVC   RACFPL31(RACFPLSL),RACFPLS
         STG   R13,ORIGR13
         LA    R13,SAVEAREA31
         XC    SAVEAREA31,SAVEAREA31
         SAM31
         STMH  R2,R14,SAVEHIGH
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF31,CLASS=CLASSBUF31,WORKA=RACFWA31,    X
               MF=(E,RACFPL31)
         LMH   R2,R14,SAVEHIGH
         SAM64
         LG    R13,ORIGR13
         L     R2,RACFPL31
         ST    R2,RACF_RC
         L     R2,RACFPL31+4
         ST    R2,RACF_RSN
         J     SAVERC

BADINPUT DS    0H
         LHI   R15,8
         J     SAVERC

NOMALL31 DS    0H
         LHI   R15,-2

SAVERC   DS    0H
         ST    R15,RETURN_CODE
         LG    R1,STOR31PTR
         LTGR  R1,R1
         JZ    SKIPFREE
         STG   R1,PARM1
         LMG   R5,R6,FREE_FD
         BASR  R7,R6
         NOPR  0
         LGR   R5,R10
SKIPFREE DS    0H
         LG    R1,RETCODEPTR
         L     R2,RACF_RC
         ST    R2,0(,R1)
         LG    R1,RSNCODEPTR
         L     R2,RACF_RSN
         ST    R2,0(,R1)
         L     R3,RETURN_CODE
         CELQEPLG ,

COPYCLASS  MVC CLASSBUF31+1(0),0(R1)
COPYENTITY MVC ENTITYBUF31+4(0),0(R1)
COPYAUTH   MVC AUTHIDB31(0),0(R1)

         DS    0D
RACFPLR  RACROUTE REQUEST=AUTH,ATTR=READ,CLASS='DUMMY',                X
               RELEASE=2.4,MF=L
RACFPLRL EQU   *-RACFPLR
RACFPLU  RACROUTE REQUEST=AUTH,ATTR=UPDATE,CLASS='DUMMY',              X
               RELEASE=2.4,MF=L
RACFPLUL EQU   *-RACFPLU
RACFPLC  RACROUTE REQUEST=AUTH,ATTR=CONTROL,CLASS='DUMMY',             X
               RELEASE=2.4,MF=L
RACFPLCL EQU   *-RACFPLC
RACFPLA  RACROUTE REQUEST=AUTH,ATTR=ALTER,CLASS='DUMMY',               X
               RELEASE=2.4,MF=L
RACFPLAL EQU   *-RACFPLA
RACFPLS  RACROUTE REQUEST=AUTH,STATUS=ACCESS,CLASS='DUMMY',            X
               RELEASE=2.4,MF=L
RACFPLSL EQU   *-RACFPLS
RACFPLTL EQU   RACFPLSL

AUTOSTG  DSECT ,
         CEEDSA SECTYPE=XPLINK

PARMLIST DS    0D
PARM1    DS    AD
PARM2    DS    AD
PARM3    DS    AD
PARM4    DS    AD
PARM5    DS    AD
PARM6    DS    AD
PARM7    DS    AD
PARM8    DS    AD
PARM9    DS    AD
PARM10   DS    AD
PARM11   DS    AD

RETURN_CODE DS F
RACF_RC  DS    F
RACF_RSN DS    F
STOR31PTR DS   AD
ORIGR13  DS    AD

DSASIZ   EQU   *-PARMLIST+CEEDSAHPSZ

MYPARMS  DSECT ,
         DS    0FD
REQPTR   DS    AD
RETCODEPTR DS  AD
RSNCODEPTR DS  AD

RACREQ_CLASSLEN EQU 0
RACREQ_CLASS EQU 4
RACREQ_ENTITYLEN EQU 12
RACREQ_ENTITY EQU 16
RACREQ_ACCESS EQU 262
RACREQ_STATUS EQU 266
RACREQ_IDTYPE EQU 270
RACREQ_AUTHIDLEN EQU 274
RACREQ_AUTHID EQU 278

STOR31   DSECT ,
         DS    0F
SAVEAREA31 DS  18F
SAVEHIGH DS    13F
         DS    0D
RACFPL31 DS    CL(RACFPLTL)
CLASSBUF31 DS  CL9
         DS    0H
ENTITYBUF31 DS H
         DS    H
         DS    CL246
AUTHIDB31 DS   CL8
         DS    0D
RACFWA31 DS    CL512
STOR31L  EQU   *-STOR31

         END   RACFAUTH