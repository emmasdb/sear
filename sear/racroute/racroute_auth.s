*---------------------------------------------------------------------*
* C bridge for RACROUTE REQUEST=AUTH.                                 *
*---------------------------------------------------------------------*
         SYSSTATE AMODE64=YES,ARCHLVL=OSREL,OSREL=SYSSTATE

RACFAUTH TITLE 'RACROUTE AUTHORIZATION'

RACFAUTH CELQPRLG DSASIZE=DSASIZ,                                      X
               BASEREG=R12,                                            X
               PARMWRDS=5,                                             X
               PARMREG=R11,                                            X
               ENTNAME=sear_racroute_auth_asm,                         X
               PSECT=RACFAUT1

sear_racroute_auth_asm ALIAS C'sear_racroute_auth_asm'
         USING AUTOSTG,R4
         USING MYPARMS,R11

         LG    R1,CLASSPTR
         L     R2,CLASSLEN
         LTGR  R1,R1
         JZ    BADINPUT
         LTR   R2,R2
         BNP   BADINPUT
         CHI   R2,8
         JH    BADINPUT

         MVI   CLASSBUF,C' '
         MVC   CLASSBUF+1(7),CLASSBUF
         LR    R3,R2
         BCTR  R3,0
         EX    R3,COPYCLASS

         LG    R1,ENTITYPTR
         L     R2,ENTITYLEN
         LTGR  R1,R1
         JZ    BADINPUT
         LTR   R2,R2
         BNP   BADINPUT
         CHI   R2,246
         JH    BADINPUT

         STH   R2,ENTITYBUF
         MVI   ENTITYBUF+2,C' '
         MVC   ENTITYBUF+3(245),ENTITYBUF+2
         LR    R3,R2
         BCTR  R3,0
         EX    R3,COPYENTITY

         L     R2,ACCESSCODE
         CHI   R2,1
         JE    AUTHREAD
         CHI   R2,2
         JE    AUTHUPDATE
         CHI   R2,3
         JE    AUTHCONTROL
         CHI   R2,4
         JE    AUTHALTER
         J     BADINPUT

AUTHREAD DS    0H
         MVC   RACFPL(RACFPLRL),RACFPLR
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               MF=(E,RACFPL)
         J     SAVERC

AUTHUPDATE DS  0H
         MVC   RACFPL(RACFPLUL),RACFPLU
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               MF=(E,RACFPL)
         J     SAVERC

AUTHCONTROL DS 0H
         MVC   RACFPL(RACFPLCL),RACFPLC
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               MF=(E,RACFPL)
         J     SAVERC

AUTHALTER DS   0H
         MVC   RACFPL(RACFPLAL),RACFPLA
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               MF=(E,RACFPL)
         J     SAVERC

BADINPUT DS    0H
         LHI   R15,8

SAVERC   DS    0H
         ST    R15,RETURN_CODE
         L     R3,RETURN_CODE
         CELQEPLG ,

COPYCLASS  MVC CLASSBUF(0),0(R1)
COPYENTITY MVC ENTITYBUF+2(0),0(R1)

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
RACFPLTL EQU   RACFPLAL

AUTOSTG  DSECT ,
         CEEDSA SECTYPE=XPLINK

PARMLIST DS    0D
PARM1    DS    AD
PARM2    DS    AD
PARM3    DS    AD
PARM4    DS    AD
PARM5    DS    AD

RETURN_CODE DS F

         DS    0D
RACFPL   DS    CL(RACFPLTL)
CLASSBUF DS    CL8
         DS    0H
ENTITYBUF DS   H
         DS    CL246
         DS    0D
RACFWA   DS    CL512

DSASIZ   EQU   *-PARMLIST+CEEDSAHPSZ

MYPARMS  DSECT ,
         DS    0FD
CLASSPTR DS    AD
CLASSLEN DS    FD
ENTITYPTR DS   AD
ENTITYLEN DS   FD
ACCESSCODE DS  FD

         END   RACFAUTH