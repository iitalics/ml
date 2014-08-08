#pragma once

/*
 * Utility vector class for a queue that you
 * can add or remove multiple items at once to,
 * efficiently
 */

#include <cstring>
#include <initializer_list>

template <typename T>
class left_vector
{
public:
	inline ~left_vector ()
	{
		delete buf_;
	}

	inline left_vector ()
		: buf_(nullptr), cap_(0), size_(0) {}
	
	left_vector (size_t initial_cap)
		: left_vector()
	{
		first_extend_(initial_cap);
	}
	
	template <typename I>
	left_vector (I begin, I end)
		: left_vector(end - begin)
	{
		insert(begin, end);
	}

	left_vector (T* array, size_t len)
		: left_vector(len)
	{
		insert(array, len);
	}

	left_vector (const std::initializer_list<T>& init)
		: left_vector(init.begin(), init.end()) {}


	template <typename I>
	void insert (I begin, I end)
	{
		size_t len = end - begin;

		extend_(len + size_);
		size_ += len;

		int j = 0;
		for (I t = begin; t != end; t++)
			buf_[cap_ - size_ + j++] = *t;
	}

	inline void insert (T* array, size_t len)
	{
		extend_(len + size_);
		size_ += len;

		memcpy(buf_ + (cap_ - size_),
		       array,
			   len * sizeof(T));
	}

	inline void erase (size_t len)
	{
		if (size_ < len)
			size_ = 0;
		else
			size_ -= len;
	}

	inline void push_front (const T& value)
	{
		extend_(++size_);
		front() = value;
	}
	inline void pop_front () { size_--; }
	inline T& front () { return buf_[cap_ - size_]; }

	inline T& operator[] (int i) { return buf_ + (cap_ - size_ + i); }
	inline T* data () { return buf_ + (cap_ - size_); }
	inline size_t size () { return size_; }

	using iterator = T*;
	inline iterator begin () { return buf_ + (cap_ - size_); }
	inline iterator end () { return buf_ + cap_; }

private:
	inline void extend_ (size_t sz)
	{
		if (sz <= cap_)
			return;
		if (cap_ == 0)
		{
			first_extend_(sz);
			return;
		}

		size_t newcap = cap_;
		
		if (cap_ == 0)
			newcap = 1;
		while (newcap < sz)
			newcap *= 2;
		
		T* newbuf = new T[newcap];
		if (size_ > 0)
			memcpy(newbuf + newcap - size_,
			       buf_ + (cap_ - size_),
				   size_ * sizeof(T));

		delete[] buf_;
		buf_ = newbuf;
		cap_ = newcap;
	}
	inline void first_extend_ (size_t sz)
	{
		size_t newcap = 1;
		
		while (newcap < sz)
			newcap *= 2;
		
		buf_ = new T[newcap];
		cap_ = newcap;
	}

	T* buf_;
	size_t cap_, size_;
};

