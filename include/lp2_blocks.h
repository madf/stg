#define setPBlockStart(qwer) asm("subb $0x20,l_"#qwer"SC1\n"\
"l_"#qwer"SC1:	.byte 0x80\n"\
"		addb $0x20,l_"#qwer"SC1\n"\
" jmp l_"#qwer"Decryptor\n\t"\
 ".string \"m_TCodeStart\"\n\t"\
  "    l_"#qwer"TCodeStart: nop\n\t")


#define setPBlockEnd(qwer) asm("		jmp l_"#qwer"Cryptor\n"\
"		.string \"m_TCodeEnd\"\n"\
"		nop\n"\
"		nop\n"\
"		nop\n"\
"		nop\n"\
"	.string \"m_PCodeEP\"\n"\
"l_"#qwer"Cryptor: 	movl d_"#qwer"Length,%edx\n"\
"		subl $4,%edx \n"\
"l_"#qwer"Cryptor_l1:  movl $28,%eax\n"\
"l_"#qwer"Cryptor_l2:  movl $d_"#qwer"Password,%ebx\n"\
"		movl (%eax,%ebx),%ecx\n"\
"		movl d_"#qwer"Start_Adr,%ebx\n"\
"		xorl %ecx,(%ebx,%edx)\n"\
"		rorl %cl,(%ebx,%edx)\n"\
"		subl $4,%edx\n"\
"		js   l_"#qwer"Cryptor_ex1\n"\
"		subl $4,%eax\n"\
"		js   l_"#qwer"Cryptor_l1\n"\
"		jmp l_"#qwer"Cryptor_l2\n"\
"l_"#qwer"Cryptor_ex1: jmp l_"#qwer"Exit\n"\
"l_"#qwer"Decryptor:   movl d_"#qwer"Length,%edx\n"\
"		subl $4,%edx \n"\
"l_"#qwer"Decryptor_l1:movl $28,%eax\n"\
"l_"#qwer"Decryptor_l2:movl $d_"#qwer"Password,%ebx\n"\
"		movl (%eax,%ebx),%ecx\n"\
"		movl d_"#qwer"Start_Adr,%ebx\n"\
"		roll %cl,(%ebx,%edx)\n"\
"		xorl %ecx,(%ebx,%edx)\n"\
"		subl $4,%edx\n"\
"		js l_"#qwer"Decryptor_ex1\n"\
"		subl $4,%eax\n"\
"		js l_"#qwer"Decryptor_l1\n"\
"		jmp l_"#qwer"Decryptor_l2\n"\
"l_"#qwer"Decryptor_ex1:jmp l_"#qwer"TCodeStart\n"\
"d_"#qwer"Start_Adr:	.string \"m_TCodeStartAdr\"\n"\
"d_"#qwer"Length:	.string \"m_TCodeLength\"\n"\
"d_"#qwer"Password:.string \"m_TCodePass\"\n"\
"           .string  \"_trfgfgfgfdfgfdfgfdfg\"\n"\
"	.string \"m_PCodeRet\"\n"\
"l_"#qwer"Exit:	addb $0x19,l_"#qwer"SC2\n"\
"l_"#qwer"SC2: 	.byte 0x48\n"\
"		subb $0x19,l_"#qwer"SC2\n")

#define DecryptROData \
asm(\
"			.string \"m_ROEP\"\n"\
"			subb $0x20,l_ro_SC1\n"\
"l_ro_SC1:		.byte 0x80\n"\
"			addb $0x20,l_ro_SC1\n"\
"l_ro_Decryptor:   	movl d_ro_Length,%edx\n"\
"			subl $4,%edx \n"\
"l_ro_Decryptor_l1:	movl $28,%eax\n"\
"l_ro_Decryptor_l2:	movl $d_ro_Password,%ebx\n"\
"			movl (%eax,%ebx),%ecx\n"\
"			movl d_ro_Start_Adr,%ebx\n"\
"			roll %cl,(%ebx,%edx)\n"\
"			subl $4,%edx\n"\
"			js l_ro_Exit\n"\
"			subl $4,%eax\n"\
"			js l_ro_Decryptor_l1\n"\
"			jmp l_ro_Decryptor_l2\n"\
"d_ro_Start_Adr:	.string \"m_ROStartAdr\"\n"\
"d_ro_Length:		.string \"m_ROLength\"\n"\
"d_ro_ExitAdr:		.string \"m_ROExitAdr\"\n"\
"d_ro_Password:		.string \"m_ROPass\"\n"\
"           		.string  \"_trfgfgfgfdfgfdfgfdfg____\"\n"\
"l_ro_Exit:		.byte 0x61\n"\
"			push d_ro_ExitAdr\n"\
"			ret\n")




