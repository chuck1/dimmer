struct Item
{
  Item(int count, void(*func)(uint8_t, uint8_t), uint8_t arg0, uint8_t arg1):
    _M_count(count),
    _M_prev(0),
    _M_next(0),
    _M_func(func),
    _M_arg0(arg0),
    _M_arg1(arg1)
  {
  }
  void    update_util(int compare, int last)
  {
    _M_tcnt = compare - (_M_count - last);
    if (_M_next) _M_next->update_util(compare, _M_count);
  }
  Item *  update(int compare, int last)
  {
    Item * ret = sort();

    ret->update_util(compare, last);

    return ret;
  }

  void		print()
  {
#ifdef __ARDUINO
    char buf[128];
    sprintf(buf, "%8i %8i %8i %8i", _M_count, _M_tcnt, _M_arg0, _M_arg1);
    Serial.println(buf);
#endif
#ifdef __LINUX
    printf("%8i %8i\n", _M_count, _M_tcnt);
#endif

    if (_M_next != 0) {
      _M_next->print();
    }
  }

  Item *		insert(Item * i)
  {
    if (i->_M_count > _M_count) {
      if (_M_next == 0) {
        _M_next = i;
        i->_M_prev = this;
      } else {
        return _M_next->insert(i);
      }
    } else {
      if (_M_prev == 0) {
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
  void		swap_prev()
  {
    // 0 - 1 - 2 - 3
    // 0 - 2 - 1 - 3

    if (_M_prev == 0) return;

    Item * i1 = _M_prev;
    Item * i0 = i1->_M_prev;
    Item * i2 = this;
    Item * i3 = _M_next;

    if (i0) i0->_M_next = i2;

    i2->_M_prev = i0;
    i2->_M_next = i1;

    i1->_M_prev = i2;
    i1->_M_next = i3;

    if (i3) i3->_M_prev = i1;
  }
  Item *		sort()
  {
    Item * ptr;// = this;

    int c = 0;

    while (true) {
      c = 0;
      ptr = seek0();
      
      while (true) {

        if (ptr == 0) break;

        if (ptr->_M_prev) {
          if (ptr->_M_count < ptr->_M_prev->_M_count) {
            ptr->swap_prev();
            ptr = ptr->_M_next->_M_next;
            c++;
            continue;
          }
        }

        ptr = ptr->_M_next;

      }
      if (c == 0) break;
    }

    return seek0();
  }
  Item *		seek0()
  {
    if (_M_prev == 0) {
      return this;
    } else {
      return _M_prev->seek0();
    }
  }
  void    call()
  {
    (*_M_func)(_M_arg0, _M_arg1);
  }

  unsigned int	_M_tcnt;
  unsigned int	_M_count;

  Item *		_M_prev;
  Item *		_M_next;

  void(*_M_func)(uint8_t, uint8_t);
  uint8_t   _M_arg0;
  uint8_t   _M_arg1;
};
