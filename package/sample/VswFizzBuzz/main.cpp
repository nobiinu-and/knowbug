
#include <cstdio>
#include <cassert>
#include <string>

// hspsdk �ւ̃p�X��ʂ��Ă����K�v�����邱�Ƃɒ���
#include "hsp3plugin.h"
#include "../../../src/ExVardataString.h"

char const* fizzBuzz(int val)
{
	if ( val % 15 == 0 ) {
		return "FizzBuzz";
	} else if ( val % 3 == 0 ) {
		return "Fizz";
	} else if ( val % 5 == 0 ) {
		return "Buzz";
	} else {
		assert(false);
	}
}

static KnowbugVswMethods const* kvswm;
EXPORT void WINAPI receiveVswMethods(KnowbugVswMethods const* vswMethods)
{
	kvswm = vswMethods;
}

EXPORT void WINAPI addValueInt(vswriter_t vsw, char const* name, void const* ptr)
{
	assert(kvswm && ptr);
	int const& val = *static_cast<int const*>(ptr);

	if ( val % 3 == 0 || val % 5 == 0 ) {
		// �uNAME : (STATE)�v�Ƃ����`�ŏo�͂���
		// catLeaf ���g��Ȃ��̂͋C�����̖��
		kvswm->catLeafExtra(vsw, name, fizzBuzz(val));

	} else {
		char buf[100];
		std::sprintf(buf, "%d\t(%08X)", val, val);

		// �uNAME = VALUE�v�Ƃ����`�ŏo�͂���
		kvswm->catLeaf(vsw, name, buf);
	}
}
