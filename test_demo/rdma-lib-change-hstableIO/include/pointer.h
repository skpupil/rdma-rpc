#include "algorithm"
#include "iostream"


template<typename T>
class SharedPtr {
  public:
    explicit SharedPtr (T* ptr = nullptr) : _pholder(new ObjectHolder(ptr)) {
    }

    SharedPtr (const SharedPtr& s) : _pholder(s._pholder) {
        this->_pholder->_pcount++;
    }

    ~SharedPtr () {
        if (--(this->_pholder->_pcount) == 0) {
            delete this->_pholder;
            this->_pholder = nullptr;
        }
    }

    SharedPtr& operator= (const SharedPtr& s){
        if (this != &s) {
            if (--(this->_pholder->_pcount) == 0) {
                delete this->_pholder;
            }
            this->_pholder = s._pholder;
            this->_pholder->_pcount++;
        }
        return *this;
    }

    T& operator*() {
        return *(this->_pholder->_ptr);
    }

    T* operator->() {
        return this->_pholder->_ptr;
    }

    operator bool () {
        return this->_pholder->_ptr;
    }

    T* get() const {
        return this->_pholder->_ptr;
    }

    void swap(SharedPtr& s) {
        std::swap(this->_pholder, s._pholder);
    }

  private:
    class ObjectHolder {
        friend class SharedPtr; // 内部类可以任意访问外部类的成员，外部类只有作为友元才能访问内部类私有成员
      public:
        explicit ObjectHolder(T* ptr = nullptr) : _ptr(ptr), _pcount(1) {
        }
        ~ObjectHolder() {
            delete _ptr;
        }
      private:
        T* _ptr;      // 指向底层对象的指针
        int _pcount;  // 引用计数，注意不是指针了
    }; // 每个对象由一个ObjectHolder持有
    ObjectHolder* _pholder; // 指向计数对象的指针
};

template<typename T>
void swap(SharedPtr<T>& a, SharedPtr<T>& b) {
    a.swap(b);
}


template <typename T>
class PointerDeleter{
  public:
    void operator() (const T *_ptr) {
        if(_ptr) {
            delete _ptr;
            _ptr = nullptr;
        }
    }
};

template <typename T, typename Deleter = PointerDeleter<T> >
class UniquePtr {
  public:
    explicit UniquePtr (T *ptr = nullptr) : _ptr(ptr) {
    }

    ~UniquePtr () {
        Deleter()(this->_ptr);
    }

    // non-copyable
    UniquePtr (const UniquePtr &p) = delete;
    UniquePtr& operator= (const UniquePtr &p) = delete;

    // move constructor
    UniquePtr (UniquePtr &&p) : _ptr(p._ptr) {
        p._ptr = nullptr;
    }

    // move assignment
    UniquePtr& operator= (UniquePtr &&p) {
        std::swap(this->_ptr, p._ptr);
        return *this;
    }

    T& operator*() {
        return *(this->_ptr);
    }

    T* operator->() {
        return this->_ptr;
    }

    operator bool () {
        return this->_ptr;
    }

    T* get() const {
        return this->_ptr;
    }

    T* release() {
        T *pointer = this->_ptr;
        this->_ptr = nullptr;
        return pointer;
    }

    void reset (T *ptr) {
        UniquePtr<T, Deleter>().swap(*this);
        this->_ptr = ptr;
    }

    void swap(UniquePtr &p) {
        std::swap(this->_ptr, p._ptr);
    }

  private:
    T *_ptr;
};

template<typename T>
void swap(UniquePtr<T>& a, UniquePtr<T>& b) {
    a.swap(b);
}




/*
int main(){

	int *a = nullptr;
	*a = 9;
	//SharedPtr<int> s1 = new SharedPtr<int>(a);	
	SharedPtr<int> s1(a);//new SharedPtr<int>(a);
	std::cout<<*s1<<std::endl;	
	return 0;
}
*/
