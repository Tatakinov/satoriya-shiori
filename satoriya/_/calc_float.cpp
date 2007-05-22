#include	"stltool.h"
#include	"simple_stack.h"



typedef	double	VALUE_TYPE;

// ���l���Z���s���Č��ʂ�String�ɁB�Ԓlfalse�Ȃ玮���ρB
// �Q�����Z�q +,-,*,/,%,^,<,>,<=,>=,==,!=,&&,||
// �P�����Z�q +,-,!
// �퉉�Z�q �����l, �J�b�R�B
// �S�Ĕ��p�ł��邱�ƁB�󔒓��͔F�߂Ȃ�
extern bool calc_float(const char* iExpression, int* oResult);
// ���p�S�p�X�y�[�X�ƃ^�u�L���̏����A�����E�L���̔��p���܂őS�����������
extern	bool calc_float(string& ioString);


struct calc_element {
	string	str;
	int		priority;
	calc_element(string _str, int _priority) : str(_str), priority(_priority) {}
	calc_element() : str(), priority(0) {}
};

static bool	make_deque(const char*& p, deque<calc_element>& oDeque) {

	while (true) {

		// �퉉�Z�q�܂��͒P�����Z�q���擾

		if ( *p == '(' ) {
			oDeque.push_back( calc_element("(", 110) );
			if ( !make_deque(++p, oDeque) )	// �J�b�R�����ċA����
				return	false;	// �G���[�̓g�b�v�܂œ`����
			if ( *p++ !=')' )
				return	false;
			oDeque.push_back( calc_element(")", 10) );
		}
		else {
			if ( !isdigit(*p) && (*p)!='.') {
				string	str;
				if ( *p=='!' ) str="!";
				else if ( *p=='+' ) str="+";
				else if ( *p=='-' ) str="-";
				else return false;	// �P�����Z�q����Ȃ��A���Ԃ���
				++p;
				oDeque.push_back( calc_element(str, 90) );
				continue;
			}

			int	len=0;
			while (isdigit(p[len]) || p[len]=='.') ++len;

			string	str(p,len);
			if ( count(str,".")>=2 )
				return	false;	// �����_���Q�ȏ゠��
			oDeque.push_back( calc_element(str, 100) );
			p+=len;
		}

		// �퉉�Z�q�̌�ɂ̂݁A����E�o
		if ( *p=='\0' || *p==')' )
			return	true;

		// �Q�����Z�q���擾

		const char*	oprs[] = { // �������̏��ɔ�r����́B
			"&&","||","==","!=","<=",">=","<",">","+","-","*","/"/*,"."*/};

		int	len=0, i;
		for (i=0 ; i<sizeof(oprs)/sizeof(oprs[0]) ; ++i) {
			len = strlen(oprs[i]);
			if ( strncmp(p, oprs[i], len) == 0 )
				break;
		}
		if ( i==sizeof(oprs)/sizeof(oprs[0]) )
			return	false;	// �ǂ̉��Z�q�ł��Ȃ�

		// ���Z�q�ɉ����ėD��x��ݒ�
		string	str(p,len);
		p+=len;
		int	priority;

		/*if ( str=="." ) { priority=85; }	// �����_
		else */if ( str=="^" ) { priority=80; }
		else if ( str=="*" || str=="/" || str=="%" ) { priority=70; }
		else if ( str=="+" || str=="-" ) { priority=60; }
		else if ( str=="<" || str==">" || str=="<=" || str==">=" ) { priority=80; }
		else if ( str=="==" || str=="!=" ) { priority=45; }
		else if ( str=="&&" ) { priority=40; }
		else if ( str=="||" ) { priority=35; }
		else return	false;

		oDeque.push_back( calc_element(str, priority) );
	}
}

// �Q�����Z
#define	a_op_b(op)	\
	else if ( el.str == #op ) {	\
		assert(stack.size()>=2); \
		VALUE_TYPE	result = stack.from_top(1) op stack.from_top(0); \
		stack.pop(2); stack.push(result); }
//�u�icalc_float,5/3�j�v
static VALUE_TYPE	calc_polish(simple_stack<calc_element>& polish) {
	simple_stack<VALUE_TYPE>	stack;
	for ( int n=0 ; n<polish.size()-1 ; n++ ) {
		calc_element&	el=polish[n];
		if ( el.priority==100 ) { // �퉉�Z�q
			stack.push( atof(el.str.c_str()) );
		}
		else if ( el.priority==90 ) {	// �P�����Z�q
			assert(stack.size()>=1);
			if ( el.str=="!" ) stack.push( !stack.pop() );
			else if ( el.str=="+" ) NULL;
			else if ( el.str=="-" ) stack.push( -stack.pop() );
			else assert(0);
		}
		/*else if ( el.priority==85 ) {	// �����_
			assert(stack.size()>=2);
			assert(el.str==".");
			char	buf[256];
			sprintf(buf, "%d.%d", int(stack.from_top(1)), int(stack.from_top(0)));
			stack.pop(2);
			stack.push( atof(buf) );
		}*/
		a_op_b(*)
		a_op_b(/)
		a_op_b(+)
		a_op_b(-)
		a_op_b(<)
		a_op_b(>)
		a_op_b(<=)
		a_op_b(>=)
		a_op_b(==)
		a_op_b(!=)
		a_op_b(&&)
		a_op_b(||)
		else 
			assert(0);

	}
	assert(stack.size()==1);
	return	stack.pop();
}

bool calc_float(const char* iExpression, VALUE_TYPE* oResult) {
	deque<calc_element>	org;
	if ( !make_deque(iExpression, org) )
		return	false;
	if ( *iExpression!='\0' )
		return	false;	// �Ȃ񂩃S�~���c���Ă��H

	simple_stack<calc_element>	stack,polish;
	stack.push(calc_element("Guard", 0));	// �ԕ�

	deque<calc_element>::const_iterator i;
	for ( i=org.begin() ; i!=org.end() ; ++i ) {
		while ( i->priority <= stack.top().priority && stack.top().str != "(" )
			polish.push(stack.pop());
		if ( i->str != ")" ) stack.push(*i); else stack.pop();
	}

	// stack����c������o��
	while ( !stack.empty() )
		polish.push(stack.pop());

	// �v�Z
	*oResult = calc_polish(polish);
	return	true;
}


bool calc_float(string& ioString) {
	erase(ioString, "�@");
	erase(ioString, " ");
	erase(ioString, "\t");
	replace(ioString, "�{", "+");
	replace(ioString, "�|", "-");
	replace(ioString, "��", "*");
	replace(ioString, "�~", "*");
	replace(ioString, "�^", "/");
	replace(ioString, "��", "/");
	replace(ioString, "��", "<");
	replace(ioString, "��", ">");
	replace(ioString, "��", "=");
	replace(ioString, "�I", "!");
	replace(ioString, "��", "&");
	replace(ioString, "�b", "|");
	replace(ioString, "�i", "(");
	replace(ioString, "�j", ")");
	replace(ioString, "�O", "0");
	replace(ioString, "�P", "1");
	replace(ioString, "�Q", "2");
	replace(ioString, "�R", "3");
	replace(ioString, "�S", "4");
	replace(ioString, "�T", "5");
	replace(ioString, "�U", "6");
	replace(ioString, "�V", "7");
	replace(ioString, "�W", "8");
	replace(ioString, "�X", "9");
	replace(ioString, "�D", ".");
	VALUE_TYPE	result;
	if ( !calc_float(ioString.c_str(), &result) )
		return	false;

	char	buf[128];
	sprintf(buf, "%f", result);
	ioString = buf;

	while ( compare_tail(ioString, "0") )
		ioString.assign(ioString.c_str(), ioString.size()-1);
	if ( compare_tail(ioString, ".") )
		ioString.assign(ioString.c_str(), ioString.size()-1);

	return	true;
}