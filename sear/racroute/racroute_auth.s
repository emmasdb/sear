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
               ENTNAME=callRauth

callRauth ALIAS C'sear_racroute_auth_asm'
         USING AUTOSTG,R4
         USING MYPARMS,R11

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

         STC   R2,CLASSBUF
         MVI   CLASSBUF+1,C' '
         MVC   CLASSBUF+2(7),CLASSBUF+1
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

         STH   R2,ENTITYBUF
         STH   R2,ENTITYBUF+2
         MVI   ENTITYBUF+4,C' '
         MVC   ENTITYBUF+5(245),ENTITYBUF+4
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

         MVI   AUTHIDB,C' '
         MVC   AUTHIDB+1(7),AUTHIDB
         LR    R3,R2
         BCTR  R3,0
         EX    R3,COPYAUTH

         MVI   NONEID,C' '
         MVC   NONEID+1(7),NONEID
         MVI   NONEID,C'*'
         MVI   NONEID+1,C'N'
         MVI   NONEID+2,C'O'
         MVI   NONEID+3,C'N'
         MVI   NONEID+4,C'E'
         MVI   NONEID+5,C'*'

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
         MVC   RACFPL(RACFPLRL),RACFPLR
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               USERID=AUTHIDB,                                         X
               MF=(E,RACFPL)
         L     R2,RACFPL
         ST    R2,RACF_RC
         L     R2,RACFPL+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHRDG DS     0H
         MVC   RACFPL(RACFPLRL),RACFPLR
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               USERID=NONEID,GROUPID=AUTHIDB,                          X
               MF=(E,RACFPL)
         L     R2,RACFPL
         ST    R2,RACF_RC
         L     R2,RACFPL+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHUPDATE DS  0H
         L     R2,RACREQ_IDTYPE(,R9)
         CHI   R2,1
         JE    AUTHUPG
         MVC   RACFPL(RACFPLUL),RACFPLU
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               USERID=AUTHIDB,                                         X
               MF=(E,RACFPL)
         L     R2,RACFPL
         ST    R2,RACF_RC
         L     R2,RACFPL+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHUPG DS     0H
         MVC   RACFPL(RACFPLUL),RACFPLU
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               USERID=NONEID,GROUPID=AUTHIDB,                          X
               MF=(E,RACFPL)
         L     R2,RACFPL
         ST    R2,RACF_RC
         L     R2,RACFPL+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHCONTROL DS 0H
         L     R2,RACREQ_IDTYPE(,R9)
         CHI   R2,1
         JE    AUTHCTG
         MVC   RACFPL(RACFPLCL),RACFPLC
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               USERID=AUTHIDB,                                         X
               MF=(E,RACFPL)
         L     R2,RACFPL
         ST    R2,RACF_RC
         L     R2,RACFPL+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHCTG DS     0H
         MVC   RACFPL(RACFPLCL),RACFPLC
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               USERID=NONEID,GROUPID=AUTHIDB,                          X
               MF=(E,RACFPL)
         L     R2,RACFPL
         ST    R2,RACF_RC
         L     R2,RACFPL+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHALTER DS   0H
         L     R2,RACREQ_IDTYPE(,R9)
         CHI   R2,1
         JE    AUTHALG
         MVC   RACFPL(RACFPLAL),RACFPLA
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               USERID=AUTHIDB,                                         X
               MF=(E,RACFPL)
         L     R2,RACFPL
         ST    R2,RACF_RC
         L     R2,RACFPL+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHALG DS     0H
         MVC   RACFPL(RACFPLAL),RACFPLA
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               USERID=NONEID,GROUPID=AUTHIDB,                          X
               MF=(E,RACFPL)
         L     R2,RACFPL
         ST    R2,RACF_RC
         L     R2,RACFPL+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHACCESS DS  0H
         L     R2,RACREQ_IDTYPE(,R9)
         CHI   R2,1
         JE    AUTHACG
         MVC   RACFPL(RACFPLSL),RACFPLS
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               USERID=AUTHIDB,                                         X
               MF=(E,RACFPL)
         L     R2,RACFPL
         ST    R2,RACF_RC
         L     R2,RACFPL+4
         ST    R2,RACF_RSN
         J     SAVERC

AUTHACG DS     0H
         MVC   RACFPL(RACFPLSL),RACFPLS
         RACROUTE REQUEST=AUTH,RELEASE=2.4,                            X
               ENTITYX=ENTITYBUF,CLASS=CLASSBUF,WORKA=RACFWA,          X
               USERID=NONEID,GROUPID=AUTHIDB,                          X
               MF=(E,RACFPL)
         L     R2,RACFPL
         ST    R2,RACF_RC
         L     R2,RACFPL+4
         ST    R2,RACF_RSN
         J     SAVERC

BADINPUT DS    0H
         LHI   R15,8

SAVERC   DS    0H
         ST    R15,RETURN_CODE
         LG    R1,RETCODEPTR
         L     R2,RACF_RC
         ST    R2,0(,R1)
         LG    R1,RSNCODEPTR
         L     R2,RACF_RSN
         ST    R2,0(,R1)
         L     R3,RETURN_CODE
         CELQEPLG ,

COPYCLASS  MVC CLASSBUF+1(0),0(R1)
COPYENTITY MVC ENTITYBUF+4(0),0(R1)
COPYAUTH   MVC AUTHIDB(0),0(R1)

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

         DS    0D
RACFPL   DS    CL(RACFPLTL)
CLASSBUF DS    CL9
         DS    0H
ENTITYBUF DS   H
         DS    H
         DS    CL246
AUTHIDB  DS    CL8
NONEID   DS    CL8
         DS    0D
RACFWA   DS    CL512

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

         END   RACFAUTH