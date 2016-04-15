#include <stdio.h>

struct Item
{
	Item(unsigned int count):
		_M_count(count),
		_M_prev(0),
		_M_next(0)
	{
	}
	
	void		print()
	{
		printf("%u\n", _M_count);

		if(_M_next != 0) {
			_M_next->print();
		}
	}

	Item *		insert(Item * i)
	{
		if(i->_M_count > _M_count) {
			if(_M_next == 0) {
				_M_next = i;
				i->_M_prev = this;
			} else {
				return _M_next->insert(i);
			}
		} else {
			if(_M_prev == 0) {
				_M_prev = i;
				i->_M_next = this;
			} else {
				_M_prev->_M_next = i;
				i->_M_prev = _M_prev;
				_M_prev = i;
				i->_M_next = this;
			}
		}

		return seek0();
	}
	Item *		seek0()
	{
		if(_M_prev == 0) {
			return this;
		} else {
			return _M_prev->seek0();
		}
	}

	unsigned int	_M_count;

	Item *		_M_prev;
	Item *		_M_next;
};


int main()
{

	Item * i0 = new Item(0);
	Item * i1 = new Item(1);
	Item * i2 = new Item(2);
	Item * i3 = new Item(3);

	i0->insert(i3);
	i0->insert(i1);
	i0->insert(i2);

	i0->print();
}


