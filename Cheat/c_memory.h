#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <atomic>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#pragma intrinsic(_ReturnAddress)
#ifndef STDCALL
#define STDCALL		__stdcall
#endif
#ifndef THISCALL
#define THISCALL	__thiscall
#endif
#ifndef FASTCALL
#define FASTCALL	__fastcall
#endif
#ifndef CDECL
#define CDECL		__cdecl
#endif
#else
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#define __forceinline __attribute__((always_inline))
#ifndef STDCALL
#define STDCALL
#endif
#ifndef THISCALL
#define THISCALL
#endif
#ifndef FASTCALL
#define FASTCALL
#endif
#ifndef CDECL
#define CDECL
#endif
#endif

namespace memory {

	// ������� ����
	using byte_t = unsigned char;
	using word_t = unsigned short;
	using dword_t = unsigned int;
	using qword_t = unsigned long long;

	// �������� �������� ������� �����
	static_assert(sizeof(byte_t) == 1, "invalid size byte");
	static_assert(sizeof(word_t) == 2, "invalid size word");
	static_assert(sizeof(dword_t) == 4, "invalid size dword");
	static_assert(sizeof(qword_t) == 8, "invalid size qword");

	// �������������� ����
	using address_t = unsigned long;
	using bytes_t = std::vector<byte_t>;

	// �������� �������� �������������� �����
	static_assert(sizeof(address_t) == sizeof(void*), "invalid size address_t");

	// ���������
	constexpr address_t null = 0u;

	// �������
	class function_t {
	private:

		// ��������� �� �������
		const void* pointer;

	public:

		function_t() : pointer(nullptr) {}
		function_t(function_t& value) { this->pointer = value.pointer; }
		template<class T> explicit function_t(T _pointer) : pointer((const void*)(_pointer)) {}

		// ��������� �����
		template<class T>
		const void* operator = (T value) {
			return this->pointer = (const void*)(value);
		}

		// �������� ����� �������
		inline const void* ptr() {
			return this->pointer;
		}

		// ����� ��� �������� ��������
		template<class... ARGS>
		inline void call(const ARGS... args) {
			(void(*)(const ARGS...))(this->address)(args...);
		}

		// ����� � ��������� ��������
		template<class ret_type, class... ARGS>
		inline ret_type call_and_return(const ARGS... args) {
			return (ret_type(*)(const ARGS...))(this->address)(args...);
		}

	};

	// �������� ������ ������
	class unprotect_scope {
	private:

		void* addr;
		dword_t size;

#ifdef _WIN32
		DWORD original_protect;
#endif

	public:

		unprotect_scope() : addr(nullptr), size(null) {};
		unprotect_scope(void* addr, const dword_t size) {
			this->addr = addr; this->size = size;
#ifdef _WIN32
			VirtualProtect(this->addr, this->size, PAGE_EXECUTE_READWRITE, &this->original_protect);
#else
			this->addr = reinterpret_cast<void*>(reinterpret_cast<long>(this->addr) & ~(sysconf(_SC_PAGE_SIZE) - 1));
			mprotect(this->addr, this->size, PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
		}

		~unprotect_scope() {
#ifdef _WIN32
			VirtualProtect(this->addr, this->size, this->original_protect, nullptr);
#else
			mprotect(this->addr, this->size, PROT_READ | PROT_EXEC);
#endif
		}

	};

	// ����������
	template<class T>
	class variable_t {
	private:

		// ��������� �� ����������
		std::atomic<T>* const pointer;

		// �������� ������ ������
		unprotect_scope scope;

	public:

		variable_t() : pointer(nullptr) {}
		variable_t(variable_t<T>& value) { this->pointer = value.pointer; }
		template<class _T> variable_t(_T _pointer) : pointer((std::atomic<T>*)(_pointer)) {
			this->scope = unprotect_scope(this->pointer, sizeof(std::atomic<T>));
		}

		// �������� ��������
		inline T get() const {
			return this->pointer->load();
		}

		// �������� ��������
		template<class _T>
		inline T set(_T value) {
			this->pointer->store((T)(value));
			return (T)(value);
		}

	};

	// ������������
	namespace hooks {

#pragma pack(push, 1)
		struct call {
			byte_t opcode = 0xE8;
			dword_t offset;
			call(const dword_t _offset) : offset(_offset) {}
			call() : offset(null) {}
		};
		struct jump {
			byte_t opcode = 0xE9;
			dword_t offset;
			jump(const dword_t _offset) : offset(_offset) {}
			jump() : offset(null) {}
		};
#pragma pack(pop)

		// ����������� ���� jump
		class jump_hook {
		private:

			bool status;
			void* inject_addr;
			void* target_addr;
			int offset;
			uint8_t original_data[sizeof(jump)];
#ifdef _WIN32
			DWORD original_protect;
#endif

		public:

			jump_hook() = delete;
			template<class T1, class T2>
			jump_hook(T1 _inject_addr, T2 _target_addr) :
				inject_addr(reinterpret_cast<void*>(_inject_addr)), target_addr(reinterpret_cast<void*>(_target_addr)) {
#ifdef _WIN32
				VirtualProtect(this->inject_addr, sizeof(jump), PAGE_EXECUTE_READWRITE, &this->original_protect);
#else
				this->inject_addr = reinterpret_cast<void*>(reinterpret_cast<long>(this->inject_addr) & ~(sysconf(_SC_PAGE_SIZE) - 1));
				mprotect(this->inject_addr, sizeof(jump), PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
				memcpy(this->original_data, this->inject_addr, sizeof(jump));
				*(jump*)(this->inject_addr) = jump(this->offset = (uint32_t)(this->target_addr) - ((uint32_t)(this->inject_addr) + sizeof(jump)));
				this->status = true;
			}

			inline void enable() {
				if (!this->status) {
					*reinterpret_cast<jump*>(this->inject_addr) = jump(this->offset);
					this->status = true;
				}
			}

			inline void disable() {
				if (this->status) {
					memcpy(this->inject_addr, this->original_data, sizeof(jump));
					this->status = false;
				}
			}

			inline void* get_original_addr() {
				return this->inject_addr;
			}

			~jump_hook() {
				disable();
#ifdef _WIN32
				VirtualProtect(this->inject_addr, sizeof(jump), this->original_protect, nullptr);
#else
				mprotect(this->inject_addr, sizeof(jump), PROT_READ | PROT_EXEC);
#endif
			}

		};

		// ����������� ���� call
		class call_hook {
		private:

			bool status;
			void* inject_addr;
			void* target_addr;
			int offset;
			uint8_t original_data[sizeof(call)];
#ifdef _WIN32
			DWORD original_protect;
#endif

		public:

			call_hook() = delete;
			template<class T1, class T2>
			call_hook(T1 _inject_addr, T2 _target_addr) :
				inject_addr(reinterpret_cast<void*>(_inject_addr)), target_addr(reinterpret_cast<void*>(_target_addr)) {
#ifdef _WIN32
				VirtualProtect(this->inject_addr, sizeof(call), PAGE_EXECUTE_READWRITE, &this->original_protect);
#else
				this->inject_addr = reinterpret_cast<void*>(reinterpret_cast<long>(this->inject_addr) & ~(sysconf(_SC_PAGE_SIZE) - 1));
				mprotect(this->inject_addr, sizeof(call), PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
				memcpy(this->original_data, this->inject_addr, sizeof(call));
				*reinterpret_cast<call*>(this->inject_addr) = call(this->offset = reinterpret_cast<uint32_t>(this->target_addr) - (reinterpret_cast<uint32_t>(this->inject_addr) + sizeof(call)));
				this->status = true;
			}

			inline void enable() {
				if (!this->status) {
					*reinterpret_cast<call*>(this->inject_addr) = call(this->offset);
					this->status = true;
				}
			}

			inline void disable() {
				if (this->status) {
					memcpy(this->inject_addr, this->original_data, sizeof(call));
					this->status = false;
				}
			}

			inline void* get_original_addr()
			{
				return this->inject_addr;
			}

			~call_hook() {
				disable();
#ifdef _WIN32
				VirtualProtect(this->inject_addr, sizeof(call), this->original_protect, nullptr);
#else
				mprotect(this->inject_addr, sizeof(call), PROT_READ | PROT_EXEC);
#endif
			}

		};

	}

	// ������ ������� ������
	class scanner {
	private:

		address_t	region_addr;
		dword_t		region_size;

	public:

		scanner() = delete;
		scanner(scanner& value) = delete;
		template<class T1, class T2>
		scanner(T1 _region_addr, T2 _region_size) :
			region_addr(reinterpret_cast<address_t>(_region_addr)), region_size(static_cast<dword_t>(_region_size)) {
			static_assert(sizeof(T1) == sizeof(address_t), "invalid scanner address size");
		}

		// ����� ������
		void* find(const char* pattern, const char* mask) {
			byte_t* current_byte = reinterpret_cast<byte_t*>(this->region_addr);
			byte_t* last_byte = reinterpret_cast<byte_t*>(this->region_addr + this->region_size - strlen(mask));
			for (dword_t i; current_byte < last_byte; current_byte++) {
				for (i = 0; static_cast<byte_t>(mask[i]); i++) {
					if (((static_cast<byte_t>(mask[i]) == static_cast<byte_t>('x')) && (static_cast<byte_t>(pattern[i]) != current_byte[i]))) break;
				} if (!static_cast<byte_t>(mask[i])) break;
			} if (current_byte == last_byte) return nullptr;
			else return current_byte;
		}

	};

	// ���������� ������
	template<unsigned long long t_signature>
	class object {
	private:

		// ��������� �������
		unsigned long long signature;

	public:

		object() { signature = t_signature; }
		virtual ~object() { signature = 0LL; }

		// �������� ��������� �� ����������
		static inline bool is_object(object* obj) {
			return obj->signature == t_signature;
		}

	};

	// ��������� �����
	class ringb {
	private:

		void(* const handler)(uint8_t*);			// ���������� ������

		uint8_t* const ptr_begin;				// ������ ������
		uint8_t* const ptr_end;				// ����� ������

		uint8_t* ptr_read;						// ��������� ������
		uint8_t* ptr_write;					// ��������� ������

		const uint32_t size_a;					// ������������ ������ ������
		uint32_t size_f;						// ������ ���������� �����

		const uint32_t size_b;					// ������ �����
		const uint32_t count_b;					// ���������� ������

	public:

		ringb(
			void(* const _handler)(uint8_t*),	// ���������� ������
			const uint32_t _size_b,				// ������ ����� (� ������)
			const uint32_t _count_b				// ���������� ������
		) :
			handler(_handler),
			size_b(_size_b),
			count_b(_count_b),
			size_a(size_f = _size_b * _count_b),
			ptr_begin((uint8_t*)malloc(_size_b* _count_b)),
			ptr_end(ptr_begin + _size_b * _count_b)
		{
			ptr_read = ptr_write = ptr_begin;
		}

		// �������� ������
		void push(
			uint8_t* data,
			uint32_t length
		) {
			if (!size_f) return;
			if (length >= size_f) {
				length = size_f;
				size_f = 0;
			}
			else size_f -= length;
			uint32_t size_right = ptr_end - ptr_write;
			if (length >= size_right) {
				memcpy(ptr_write, data, size_right);
				if (length -= size_right) {
					memcpy(ptr_write = ptr_begin, data + size_right, length);
					ptr_write += length;
				}
			}
			else {
				memcpy(ptr_write, data, length);
				ptr_write += length;
			} if (size_a - size_f >= size_b) {
				size_f += size_b; handler(ptr_read);
				if ((ptr_read += size_b) >= ptr_end)
					ptr_read = ptr_begin;
			}
		}

		~ringb() {
			std::free(ptr_begin);
		}

	};

	// �������� ���������� � ������ ������
	static bool get_module_info(
		void* t_addr,
		void*& m_addr,
		dword_t& m_size
	) {
#ifdef _WIN32					
		MEMORY_BASIC_INFORMATION info;
		if (!VirtualQuery(reinterpret_cast<void*>(t_addr), &info, sizeof(info))) return false;
		auto pe = (IMAGE_NT_HEADERS*)((memory::address_t)(info.AllocationBase) + ((IMAGE_DOS_HEADER*)(info.AllocationBase))->e_lfanew);
		if (pe->Signature != IMAGE_NT_SIGNATURE) return false;
		if (!(m_size = pe->OptionalHeader.SizeOfImage)) return false;
		m_addr = info.AllocationBase;
#else
		Dl_info info{};
		struct stat buf {};
		if (!dladdr(t_addr, &info)) return false;
		m_addr = info.dli_fbase;
		if (!(m_size = getfilesize(info.dli_fname)))
			return false;
#endif
		return true;
	}

	// �������� ����� �������� ������� �������
	static __forceinline void* get_ret_addr() {
		void* ret_addr;
#ifdef _WIN32
		__asm {
			push eax
			mov eax, dword ptr[ebp + 4]
			mov ret_addr, eax
			pop eax
		}
#else
		__asm__(
			".intel_syntax noprefix\n\t"
			"mov eax, dword ptr [ebp + 4]\n\t"
			"mov %0, eax\n\t"
			: "=a"(ret_addr)
			:: "%eax"
		);
#endif
		return ret_addr;

	}

	// ������� ���������� ������
	// (������������ ����������� 10.0 � ��������� �� 0 �� 100 000)
	static inline float qsqrt(float number) {
		float result;
#ifdef _WIN32
		__asm {
			mov eax, number
			sub eax, 0x3f800000
			sar eax, 1
			add eax, 0x3f800000
			mov result, eax
		}
#else
		__asm__(
			".intel_syntax noprefix\n\t"
			"mov eax, %1\n\t"
			"sub eax, 0x3f800000\n\t"
			"sar eax, 1\n\t"
			"add eax, 0x3f800000\n\t"
			"mov %0, eax\n\t"
			: "=a"(result)
			: "a"(number)
			: "%eax"
		);
#endif
		return result;
	}

}

#endif