// This file is generated, don't edit it!!

#if !defined(ERRHEADER_XMLExcepts)
#define ERRHEADER_XMLExcepts

class XMLExcepts
{
public :
    enum Codes
    {
        NoError                            = 0
      , E_LowBounds                        = 1
      , Array_BadIndex                     = 2
      , Array_BadNewSize                   = 3
      , AttrList_BadIndex                  = 4
      , AttDef_BadAttType                  = 5
      , AttDef_BadDefAttType               = 6
      , Bitset_BadIndex                    = 7
      , Bitset_NotEqualSize                = 8
      , BufMgr_NoMoreBuffers               = 9
      , BufMgr_BufferNotInPool             = 10
      , CPtr_PointerIsZero                 = 11
      , CM_BinOpHadUnaryType               = 12
      , CM_MustBeMixedOrChildren           = 13
      , CM_NoPCDATAHere                    = 14
      , CM_NotValidForSpecType             = 15
      , CM_UnaryOpHadBinType               = 16
      , CM_UnknownCMType                   = 17
      , CM_UnknownCMSpecType               = 18
      , CM_NoParentCSN                     = 19
      , CM_NotValidSpecTypeForNode         = 20
      , DTD_UnknownCreateReason            = 21
      , ElemStack_EmptyStack               = 22
      , ElemStack_BadIndex                 = 23
      , ElemStack_StackUnderflow           = 24
      , ElemStack_NoParentPushed           = 25
      , Enum_NoMoreElements                = 26
      , File_CouldNotOpenFile              = 27
      , File_CouldNotGetCurPos             = 28
      , File_CouldNotCloseFile             = 29
      , File_CouldNotSeekToEnd             = 30
      , File_CouldNotSeekToPos             = 31
      , File_CouldNotDupHandle             = 32
      , File_CouldNotReadFromFile          = 33
      , File_CouldNotResetFile             = 34
      , File_CouldNotGetSize               = 35
      , File_CouldNotGetBasePathName       = 36
      , File_BasePathUnderflow             = 37
      , Gen_ParseInProgress                = 38
      , Gen_NoDTDValidator                 = 39
      , Gen_CouldNotOpenDTD                = 40
      , Gen_CouldNotOpenExtEntity          = 41
      , Gen_UnexpectedEOF                  = 42
      , HshTbl_ZeroModulus                 = 43
      , HshTbl_BadHashFromKey              = 44
      , HshTbl_NoSuchKeyExists             = 45
      , Mutex_CouldNotCreate               = 46
      , Mutex_CouldNotClose                = 47
      , Mutex_CouldNotLock                 = 48
      , Mutex_CouldNotUnlock               = 49
      , Mutex_CouldNotDestroy              = 50
      , NetAcc_InternalError               = 51
      , NetAcc_InitFailed                  = 52
      , NetAcc_TargetResolution            = 53
      , NetAcc_CreateSocket                = 54
      , NetAcc_ConnSocket                  = 55
      , NetAcc_WriteSocket                 = 56
      , NetAcc_ReadSocket                  = 57
      , Pool_ElemAlreadyExists             = 58
      , Pool_BadHashFromKey                = 59
      , Pool_InvalidId                     = 60
      , Pool_ZeroModulus                   = 61
      , RdrMgr_ReaderIdNotFound            = 62
      , Reader_BadAutoEncoding             = 63
      , Reader_CouldNotDecodeFirstLine     = 64
      , Reader_EOIInMultiSeq               = 65
      , Reader_SrcOfsNotSupported          = 66
      , Reader_EncodingStrRequired         = 67
      , Scan_CouldNotOpenSource            = 68
      , Scan_UnbalancedStartEnd            = 69
      , Scan_BadPScanToken                 = 70
      , Stack_BadIndex                     = 71
      , Stack_EmptyStack                   = 72
      , Str_ZeroSizedTargetBuf             = 73
      , Str_UnknownRadix                   = 74
      , Str_TargetBufTooSmall              = 75
      , Str_StartIndexPastEnd              = 76
      , Str_ConvertOverflow                = 77
      , Strm_StdErrWriteFailure            = 78
      , Strm_StdOutWriteFailure            = 79
      , Strm_ConWriteFailure               = 80
      , StrPool_IllegalId                  = 81
      , Trans_CouldNotCreateDefCvtr        = 82
      , Trans_InvalidSizeReq               = 83
      , Trans_Unrepresentable              = 84
      , Trans_NotValidForEncoding          = 85
      , Trans_BadBlockSize                 = 86
      , Trans_BadSrcSeq                    = 87
      , Trans_BadSrcCP                     = 88
      , Trans_BadTrailingSurrogate         = 89
      , Trans_CantCreateCvtrFor            = 90
      , URL_MalformedURL                   = 91
      , URL_UnsupportedProto               = 92
      , URL_UnsupportedProto1              = 93
      , URL_OnlyLocalHost                  = 94
      , URL_NoProtocolPresent              = 95
      , URL_ExpectingTwoSlashes            = 96
      , URL_IncorrectEscapedCharRef        = 97
      , URL_UnterminatedHostComponent      = 98
      , URL_RelativeBaseURL                = 99
      , URL_BaseUnderflow                  = 100
      , URL_BadPortField                   = 101
      , Vector_BadIndex                    = 102
      , Val_InvalidElemId                  = 103
      , Val_CantHaveIntSS                  = 104
      , XMLRec_UnknownEncoding             = 105
      , Parser_Parse1                      = 106
      , Parser_Parse2                      = 107
      , Parser_Next1                       = 108
      , Parser_Next2                       = 109
      , Parser_Next3                       = 110
      , Parser_Next4                       = 111
      , Parser_Factor1                     = 112
      , Parser_Factor2                     = 113
      , Parser_Factor3                     = 114
      , Parser_Factor4                     = 115
      , Parser_Factor5                     = 116
      , Parser_Factor6                     = 117
      , Parser_Atom1                       = 118
      , Parser_Atom2                       = 119
      , Parser_Atom3                       = 120
      , Parser_Atom4                       = 121
      , Parser_Atom5                       = 122
      , Parser_CC1                         = 123
      , Parser_CC2                         = 124
      , Parser_CC3                         = 125
      , Parser_CC4                         = 126
      , Parser_CC5                         = 127
      , Parser_CC6                         = 128
      , Parser_CC7                         = 129
      , Parser_Ope1                        = 130
      , Parser_Ope2                        = 131
      , Parser_Descape1                    = 132
      , Parser_Descape2                    = 133
      , Parser_Descape3                    = 134
      , Parser_Descape4                    = 135
      , Parser_Descape5                    = 136
      , Parser_Process1                    = 137
      , Gen_NoSchemaValidator              = 138
      , XUTIL_UnCopyableNodeType           = 139
      , SubGrpComparator_NGR               = 140
      , FACET_Invalid_Len                  = 141
      , FACET_Invalid_maxLen               = 142
      , FACET_Invalid_minLen               = 143
      , FACET_NonNeg_Len                   = 144
      , FACET_NonNeg_maxLen                = 145
      , FACET_NonNeg_minLen                = 146
      , FACET_Len_maxLen                   = 147
      , FACET_Len_minLen                   = 148
      , FACET_maxLen_minLen                = 149
      , FACET_bool_Pattern                 = 150
      , FACET_Invalid_Tag                  = 151
      , FACET_Len_baseLen                  = 152
      , FACET_minLen_baseminLen            = 153
      , FACET_minLen_basemaxLen            = 154
      , FACET_maxLen_basemaxLen            = 155
      , FACET_maxLen_baseminLen            = 156
      , FACET_enum_base                    = 157
      , FACET_Invalid_WS                   = 158
      , FACET_WS_collapse                  = 159
      , FACET_WS_replace                   = 160
      , FACET_Invalid_MaxIncl              = 161
      , FACET_Invalid_MaxExcl              = 162
      , FACET_Invalid_MinIncl              = 163
      , FACET_Invalid_MinExcl              = 164
      , FACET_Invalid_TotalDigit           = 165
      , FACET_Invalid_FractDigit           = 166
      , FACET_NonNeg_TotalDigit            = 167
      , FACET_NonNeg_FractDigit            = 168
      , FACET_max_Incl_Excl                = 169
      , FACET_min_Incl_Excl                = 170
      , FACET_maxExcl_minExcl              = 171
      , FACET_maxExcl_minIncl              = 172
      , FACET_maxIncl_minExcl              = 173
      , FACET_maxIncl_minIncl              = 174
      , FACET_TotDigit_FractDigit          = 175
      , FACET_maxIncl_base_maxExcl         = 176
      , FACET_maxIncl_base_maxIncl         = 177
      , FACET_maxIncl_base_minIncl         = 178
      , FACET_maxIncl_base_minExcl         = 179
      , FACET_maxExcl_base_maxExcl         = 180
      , FACET_maxExcl_base_maxIncl         = 181
      , FACET_maxExcl_base_minIncl         = 182
      , FACET_maxExcl_base_minExcl         = 183
      , FACET_minExcl_base_maxExcl         = 184
      , FACET_minExcl_base_maxIncl         = 185
      , FACET_minExcl_base_minIncl         = 186
      , FACET_minExcl_base_minExcl         = 187
      , FACET_minIncl_base_maxExcl         = 188
      , FACET_minIncl_base_maxIncl         = 189
      , FACET_minIncl_base_minIncl         = 190
      , FACET_minIncl_base_minExcl         = 191
      , FACET_maxIncl_notFromBase          = 192
      , FACET_maxExcl_notFromBase          = 193
      , FACET_minIncl_notFromBase          = 194
      , FACET_minExcl_notFromBase          = 195
      , FACET_totalDigit_base_totalDigit   = 196
      , FACET_fractDigit_base_totalDigit   = 197
      , FACET_fractDigit_base_fractDigit   = 198
      , VALUE_NotMatch_Pattern             = 199
      , VALUE_Not_Base64                   = 200
      , VALUE_Not_HexBin                   = 201
      , VALUE_GT_maxLen                    = 202
      , VALUE_LT_minLen                    = 203
      , VALUE_NE_Len                       = 204
      , VALUE_NotIn_Enumeration            = 205
      , VALUE_exceed_totalDigit            = 206
      , VALUE_exceed_fractDigit            = 207
      , VALUE_exceed_maxIncl               = 208
      , VALUE_exceed_maxExcl               = 209
      , VALUE_exceed_minIncl               = 210
      , VALUE_exceed_minExcl               = 211
      , VALUE_WS_replaced                  = 212
      , VALUE_WS_collapsed                 = 213
      , XMLNUM_emptyString                 = 214
      , XMLNUM_WSString                    = 215
      , XMLNUM_2ManyDecPoint               = 216
      , XMLNUM_Inv_chars                   = 217
      , XMLNUM_null_ptr                    = 218
      , Regex_Result_Not_Set               = 219
      , Regex_CompactRangesError           = 220
      , Regex_MergeRangesTypeMismatch      = 221
      , Regex_SubtractRangesError          = 222
      , Regex_IntersectRangesError         = 223
      , Regex_ComplementRangesInvalidArg   = 224
      , Regex_InvalidCategoryName          = 225
      , Regex_KeywordNotFound              = 226
      , Regex_BadRefNo                     = 227
      , Regex_UnknownOption                = 228
      , Regex_UnknownTokenType             = 229
      , Regex_RangeTokenGetError           = 230
      , Regex_NotSupported                 = 231
      , Regex_InvalidChildIndex            = 232
      , NEL_RepeatedCalls                  = 233
      , RethrowError                       = 234
      , Out_Of_Memory                      = 235
      , E_HighBounds                       = 236
      , W_LowBounds                        = 237
      , W_HighBounds                       = 238
      , F_LowBounds                        = 239
      , F_HighBounds                       = 240
    };

};
#endif

