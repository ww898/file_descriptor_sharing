#pragma once

#include <utility>

namespace jetbrains {
namespace common {

template<typename _Ht, typename _Dt, _Ht _Ev = _Ht()>
struct unique_hld final
{
	typedef unique_hld<_Ht, _Dt, _Ev> _Myt;

	unique_hld() throw() :
		_Myhdl(_Ev)
	{
	}

	unique_hld(_Ht _Hdl) throw() :
		_Myhdl(_Hdl)
	{
	}

	unique_hld(_Ht _Hdl, _Dt _Del) throw() :
		_Myhdl(_Hdl),
		_Mydel(_Del)
	{
	}

	~unique_hld() throw()
	{
		if (_Myhdl != _Ev)
			_Mydel(_Myhdl);
	}

	bool empty() const throw() { return _Myhdl == _Ev; }
	explicit operator bool() const throw() { return _Myhdl != _Ev; }
	_Ht get() const throw() { return _Myhdl; }

	_Ht * reset_by_ref() throw()
	{
		reset();
		return &_Myhdl;
	}

	void reset(_Ht _Hdl = _Ev) throw()
	{
		_Ht _OldHdl = _Myhdl;
		_Myhdl = _Hdl;
		if (_OldHdl != _Ev)
			_Mydel(_OldHdl);
	}

	_Ht release() throw()
	{
		_Ht _Hdl = _Myhdl;
		_Myhdl = _Ev;
		return _Hdl;
	}

	void swap(_Myt & _Right) throw()
	{
		std::swap(_Myhdl, _Right._Myhdl);
		std::swap(_Mydel, _Right._Mydel);
	}

	unique_hld(_Myt const &) = delete;
	_Myt & operator=(_Myt const &) = delete;

	unique_hld(_Myt && _Right) throw() :
		_Myhdl(_Ev)
	{
		swap(_Right);
	}

	_Myt & operator=(_Myt && _Right) throw()
	{
		swap(_Right);
		return *this;
	}

private:
	_Ht _Myhdl;
	_Dt _Mydel;
};

}}
