#pragma once

#include <iostream> //temp for debug
#include <iomanip>
#include <vector>
#include <type_traits>
#include <intrin.h>

namespace Jbonk::Math
{
	/*
	ROW MAJOR
	   _n_n_n_
	m | 0 1 2 |
	m | 3 4 5 | iTh row i = 1
	m |_6_7_8_|
		jTh column j = 0
	*/
	
	template<typename T>
	concept FloatType = std::floating_point<T>;

	template<class Matrix>
	class Matrix_const_iterator
	{
	public:
		using Value_Type = typename Matrix::value_type;
		using Pointer_Type = typename Matrix::const_pointer;
		using Reference_Type = Value_Type&;
		using _Tptr = typename Matrix::pointer;
	public:
		constexpr Matrix_const_iterator() noexcept : _Ptr(nullptr) {}

		constexpr Matrix_const_iterator(_Tptr ptr) noexcept : _Ptr(ptr) {}

		constexpr Reference_Type operator*() const noexcept
		{
			return *_Ptr;
		}

		constexpr Pointer_Type operator->() const noexcept
		{
			return _Ptr;
		}

		constexpr Reference_Type operator[](const size_t index) const noexcept
		{
			return *(_Ptr + index);
		}

		constexpr Matrix_const_iterator& operator+=(const size_t offset) noexcept
		{
			_Ptr += offset;
			return *this;
		}

		constexpr Matrix_const_iterator& operator-=(const size_t offset) noexcept
		{
			return *this += -offset;
		}

		constexpr Matrix_const_iterator operator+(const size_t offset) const noexcept
		{
			Matrix_const_iterator temp = *this;
			temp += offset;
			return temp;
		}

		friend constexpr Matrix_const_iterator operator+(const size_t offset, Matrix_const_iterator right) noexcept
		{
			right += offset;
			return right;
		}

		constexpr Matrix_const_iterator operator-(const size_t offset) const noexcept
		{
			Matrix_const_iterator temp = *this;
			temp -= offset;
			return temp;
		}

		constexpr size_t operator-(const Matrix_const_iterator& right) const noexcept
		{
			return _Ptr - right._Ptr;
		}

		constexpr Matrix_const_iterator& operator++() noexcept
		{
			++_Ptr;
			return *this;
		}

		constexpr Matrix_const_iterator operator++(int) noexcept
		{
			Matrix_const_iterator temp = *this;
			++(*this);
			return temp;
		}

		constexpr Matrix_const_iterator& operator--() noexcept
		{
			--_Ptr;
			return *this;
		}

		constexpr Matrix_const_iterator operator--(int) noexcept
		{
			Matrix_const_iterator temp = *this;
			--(*this);
			return temp;
		}

		constexpr bool operator==(const Matrix_const_iterator& right) const noexcept
		{
			return _Ptr == right._Ptr;
		}

		constexpr bool operator!=(const Matrix_const_iterator& right) const noexcept
		{
			return !(*this == right);
		}

	private:
		_Tptr _Ptr;
	};

	template<class Matrix>
	class Matrix_iterator : public Matrix_const_iterator<Matrix>
	{
	public:
		using _MyBase = Matrix_const_iterator<Matrix>;

		using Value_Type = typename Matrix::value_type;
		using Pointer_Type = Value_Type*;
		using Reference_Type = Value_Type&;

		using _MyBase::_MyBase;
	public:

		constexpr Reference_Type operator*() const noexcept
		{
			return const_cast<Reference_Type>(_MyBase::operator*());
		}

		constexpr Pointer_Type operator->() const noexcept
		{
			return this->_Ptr;
		}

		constexpr Reference_Type operator[](const size_t index) const noexcept
		{
			return const_cast<Reference_Type>(_MyBase::operator[](index));
		}

		constexpr Matrix_iterator& operator+=(const size_t offset) noexcept
		{
			_MyBase::operator+=(offset);
			return *this;
		}

		constexpr Matrix_iterator& operator-=(const size_t offset) noexcept
		{
			_MyBase::operator-=(offset);
			return *this += -offset;
		}

		constexpr Matrix_iterator operator+(const size_t offset) const noexcept
		{
			Matrix_iterator temp = *this;
			temp += offset;
			return temp;
		}

		friend constexpr Matrix_iterator operator+(const size_t offset, Matrix_iterator right)
		{
			right += offset;
			return right;
		}

		constexpr Matrix_iterator operator-(const size_t offset) const noexcept
		{
			Matrix_iterator temp = *this;
			temp -= offset;
			return temp;
		}

		constexpr Matrix_iterator& operator++() noexcept
		{
			_MyBase::operator++();
			return *this;
		}

		constexpr Matrix_iterator operator++(int) noexcept
		{
			Matrix_iterator temp = *this;
			_MyBase::operator++();
			return temp;
		}

		constexpr Matrix_iterator& operator--() noexcept
		{
			_MyBase::operator--();
			return *this;
		}

		constexpr Matrix_iterator operator--(int) noexcept
		{
			Matrix_iterator temp = *this;
			_MyBase::operator--();
			return temp;
		}

	private:
		Pointer_Type _Ptr;
	};

	template<FloatType T>
	class Matrix
	{
	public:
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const pointer;
		using reference = value_type&;
		using const_reference = const reference;
		using iterator = Matrix_iterator<Matrix<T>>;
		using const_iterator = Matrix_const_iterator<Matrix<T>>;
	public:
		explicit Matrix() noexcept : _data(nullptr), _rows(0), _columns(0) {}

		explicit Matrix(size_t rows, size_t columns, value_type value = value_type()) noexcept
			: _data(nullptr), _rows(rows), _columns(columns)
		{
			Initialize(rows, columns, value);
		}

		Matrix(const Matrix& other) noexcept
		{
			_rows = other._rows;
			_columns = other._columns;

			_data = std::make_unique<value_type[]>(other._rows * other._columns);

			for (size_t i = 0; i < _rows; i++)
			{
				for (size_t j = 0; j < _columns; j++)
				{
					(*this)[i][j] = other[i][j];
				}
			}
		}

		Matrix(Matrix&& other) noexcept
			: _rows(other._rows), _columns(other._columns), _data(std::move(other._data))
		{
		}

		~Matrix() = default;

		constexpr const size_t Rows() const { return _rows; }
		constexpr const size_t Columns() const { return _columns; }
		constexpr const size_t Size() const { return _rows * _columns; }
		constexpr iterator begin() { return iterator(_data.get()); }
		constexpr iterator end() { return iterator(_data.get() + Size()); }
		constexpr const_iterator begin() const { return const_iterator(_data.get()); }
		constexpr const_iterator end() const { return const_iterator(_data.get() + Size()); }
		constexpr const_iterator cbegin() const { begin(); }
		constexpr const_iterator cend() const { end(); }

		//operators

		Matrix& operator=(const Matrix& other)
		{
			_rows = other._rows;
			_columns = other._columns;

			_data = std::make_unique<value_type[]>(other.Size());

			for (size_t i = 0; i < _rows; i++)
			{
				for (size_t j = 0; j < _columns; j++)
				{
					(*this)[i][j] = other[i][j];
				}
			}

			return *this;
		}

		constexpr const pointer operator[](const size_t row) const noexcept						
		{
			JB_CORE_ASSERT(row < _rows);
			return row * _columns + _data.get(); 
		}

		constexpr pointer operator[](const size_t row) noexcept
		{
			JB_CORE_ASSERT(row < _rows); 
			return row * _columns + _data.get(); 
		}

		constexpr reference operator()(const size_t index) noexcept
		{
			return _data[index];
		}

		constexpr const_reference operator()(const size_t index) const noexcept
		{
			return _data[index];
		}

		constexpr const reference operator()(const size_t row, const size_t column) const noexcept
		{
			JB_CORE_ASSERT(row < _rows || column < _columns);
			return _data[row * _columns + column]; 
		}
		//TODO: all of this is so stupid, I dont like operator overloading anymore so lets just remove this later
		Matrix operator*(Matrix& B) { return Multiply(B); }
		Matrix operator*(value_type scale) { return Scale(*this, scale); }
		Matrix& operator+=(const Matrix& B) { return AddToThis(B); }
		Matrix& operator-=(const Matrix& B) { return AddMinusToThis(B); }
		Matrix& operator*=(const Matrix& B) { return MultiplyToThis(B); }
		Matrix& operator*=(value_type scale) { return MultiplyToThis(scale); }


		//Debug methods
		void Print() 
		{
			int round = 3;
			int width = 8;
			const double pow10 = std::pow(10.0, round);
			std::cout << "----------------------------------------------\n";
			for (size_t i = 0; i < _rows; i++)
			{
				for (size_t j = 0; j < _columns; j++)
				{
					std::cout << std::setprecision(round) << std::fixed << std::setw(width) << std::right << (std::round((*this)[i][j] * pow10) / pow10) << " ";
				}
				std::cout << '\n';
			}
			std::cout << "----------------------------------------------\n";
		}
	
		//Matrix methods
		void Resize(size_t rows, size_t columns)
		{
			if (rows == _rows && columns == _columns)
				return;

			_data = std::make_unique<value_type[]>(rows * columns);
			_rows = rows;
			_columns = columns;
		}

		void Initialize(size_t rows, size_t columns, value_type value = value_type())
		{
			_data = std::make_unique<value_type[]>(rows * columns);
			_rows = rows;
			_columns = columns;

			for (size_t i = 0; i < Size(); i++)
				_data[i] = value;
		}

		void InsertMatrix(size_t row, size_t column, const Matrix& subMatrix)
		{
			JB_CORE_ASSERT(row < _rows && column < _columns);

			for (size_t i = 0; i < subMatrix._rows; i++)
			{
				for (size_t j = 0; j < subMatrix._columns; j++)
				{
					(*this)[i + row][j + column] = subMatrix[i][j];
				}
			}
		}

		//Matrix operations
		Matrix ScaleLeftDiagonal(const Matrix& vector)
		{
			JB_CORE_ASSERT(vector._columns < 2);
			JB_CORE_ASSERT(vector._rows == this->_rows);

			Matrix<value_type> output = *this;

			for (size_t i = 0; i < _rows; i++)
			{
				for (size_t j = 0; j < _columns; j++)
				{
					output[i][j] *= vector[i][0];
				}
			}

			return output;
		}

		Matrix Transpose()
		{
			Matrix<value_type> output(_columns, _rows);

			for (size_t i = 0; i < _rows; i++)
			{
				for (size_t j = 0; j < _columns; j++)
				{
					output[j][i] = (*this)[i][j];
				}
			}

			return output;
		}

		Matrix Multiply(const Matrix& B)
		{
			JB_CORE_ASSERT(_columns == B._rows); //AB so width of A must equal Height of B
			Matrix<value_type> output(_rows, B._columns);

			for (size_t i = 0; i < _rows; i++)
			{
				for (size_t j = 0; j < B._columns; j++)
				{
					value_type v = value_type();
					for (size_t k = 0; k < _columns; k++)
						v += (*this)[i][k] * B[k][j];

					output[i][j] = v;
				}
			}
			return output;
		}

		Matrix& MultiplyToThis(const Matrix& B)
		{
			JB_CORE_ASSERT(_columns == B._rows); //AB so width of A must equal Height of B

			Resize(_rows, B._columns);

			for (size_t i = 0; i < _rows; i++)
			{
				for (size_t j = 0; j < B._columns; j++)
				{
					value_type v = value_type();
					for (size_t k = 0; k < _columns; k++)
						v += (*this)[i][k] * B[k][j];

					(*this)[i][j] = v;
				}
			}

			return *this;
		}

		Matrix& MultiplyToThis(value_type scale)
		{
			for (size_t i = 0; i < _rows; i++)
			{
				for (size_t j = 0; j < _columns; j++)
				{
					(*this)[i][j] *= scale;
				}
			}

			return *this;
		}

		//Vector operations
		value_type Magnitude() const
		{
			JB_CORE_ASSERT(this->_columns == 1);

			value_type mag = value_type();
			for (size_t i = 0; i < _rows; i++)
			{
				mag += (*this)[i][0] * (*this)[i][0];
			}

			return glm::sqrt(mag);
		}

		value_type MagnitudeSquared() const
		{
			JB_CORE_ASSERT(this->_columns == 1);

			value_type mag = value_type();
			for (size_t i = 0; i < _rows; i++)
			{
				mag += (*this)[i][0] * (*this)[i][0];
			}

			return mag;
		}

		value_type Dot(const Matrix& b) const
		{
			JB_CORE_ASSERT(this->_columns == 1);
			JB_CORE_ASSERT(this->_columns == b._columns);
			JB_CORE_ASSERT(this->_rows == b._rows);

			value_type dot = 0.0;
			for (size_t i = 0; i < _rows; i++)
			{
				dot += (*this)[i][0] * b[i][0];
			}

			return dot;
		}

		//Matrix and vector
		Matrix& AddToThis(const Matrix& B)
		{
			JB_CORE_ASSERT(this->_rows == B._rows && this->_columns == B._columns);

			for (size_t i = 0; i < _rows; i++)
			{
				for (size_t j = 0; j < _columns; j++)
				{
					(*this)[i][j] += B[i][j];
				}
			}

			return *this;
		}

		Matrix& AddMinusToThis(const Matrix& B)
		{
			JB_CORE_ASSERT(this->_rows == B._rows && this->_columns == B._columns);

			for (size_t i = 0; i < _rows; i++)
			{
				for (size_t j = 0; j < _columns; j++)
				{
					(*this)[i][j] -= B[i][j];
				}
			}

			return *this;
		}

		Matrix Scale(const Matrix& B, value_type scale)
		{
			Matrix<value_type> scaled(_rows, _columns);
			for (size_t i = 0; i < _rows; i++)
			{
				for (size_t j = 0; j < _columns; j++)
				{
					scaled[i][j] = scale * B[i][j];
				}
			}

			return scaled;
		}


		//static methods
		static void Multiply(const Matrix& A, const Matrix& B, Matrix& C)
		{
			JB_CORE_ASSERT(A._columns == B._rows);

			C.Resize(A._rows, B._columns);

			for (size_t i = 0; i < A._rows; i++)
			{
				for (size_t j = 0; j < B._columns; j++)
				{
					value_type v = 0.0;
					for (size_t k = 0; k < A._columns; k++)
						v += A[i][k] * B[k][j];

					C[i][j] = v;
				}
			}
		}

		static void Multiply(const Matrix& A, value_type scale, Matrix& C)
		{
			for (size_t i = 0; i < A._rows; i++)
			{
				for (size_t j = 0; j < A._columns; j++)
				{
					C[i][j] = scale * A[i][j];
				}
			}
		}

		static void ScaleLeftDiagonal(const Matrix& A, const Matrix& diagonal, Matrix& C)
		{
			JB_CORE_ASSERT(diagonal._columns < 2);
			JB_CORE_ASSERT(diagonal._rows == A._rows);

			C.Resize(A._rows, A._columns);

			for (size_t i = 0; i < A._rows; i++)
			{
				for (size_t j = 0; j < A._columns; j++)
				{
					C[i][j] = diagonal[i][0] * A[i][j];
				}
			}
		}

	private:
		size_t _rows, _columns;
		std::unique_ptr<value_type[]> _data;
	};

	
	template<FloatType T>
	class SparseMatrix
	{
	public:
		SparseMatrix(uint16_t rows, uint16_t columns);
		T get(size_t row, size_t column) const;
		void set(size_t row, size_t column, const T& value);
		void update_row_indices(size_t row);
	private:
		uint16_t _rows, _columns;

		std::vector<T> _nnz_values;
		std::vector<size_t> _col_index;
		std::vector<size_t> _row_index;

	};

	template<FloatType T>
	inline SparseMatrix<T>::SparseMatrix(uint16_t rows, uint16_t columns)
		: _rows(rows), _columns(columns)
	{
		_row_index = std::vector<size_t>(rows + 1, 0);
	}

	template<FloatType T>
	inline void SparseMatrix<T>::update_row_indices(size_t row)
	{
		if (row == _rows - 1)
			_row_index[row + 1]++;

		for (size_t i = _row_index[row]; i < _row_index.size(); i++)
			_row_index[i]++;
	}

	template<FloatType T>
	inline void SparseMatrix<T>::set(size_t row, size_t column, const T& value)
	{
		if (value == T())
			return;

		if (_nnz_values.empty())
			return;

		size_t row_start = _row_index[row];
		size_t row_end = _row_index[row + 1] - 1;

		if (_col_index[row_start] == column)
		{
			_nnz_values[row_start] = value;
			return;
		}

		if (_col_index[row_end] < column)
		{
			_nnz_values.insert(_nnz_values.begin() + row_end + 1, value);
			_col_index.insert(_col_index.begin() + row_end + 1, column);
			update_row_indices(row);
			return;
		}

		if (_col_index[row_start] > column)
		{
			_nnz_values.insert(_nnz_values.begin() + row_start, value);
			_col_index.insert(_col_index.begin() + row_start, column);
			update_row_indices(row);
			return;
		}
		
		for (size_t i = _col_index[row_start]; i < _col_index[row_end] + 1; i++)
		{
			if (i == column)
			{
				_nnz_values.insert(_nnz_values.begin() + i + 2, value);
				_col_index.insert(_col_index.begin() + i + 2, column);
				update_row_indices(row);
				return;
			}
		}
	}


	template<FloatType T>
	inline T SparseMatrix<T>::get(size_t row, size_t column) const
	{
		size_t row_start = _row_index[row];
		size_t row_end = _row_index[row + 1];
		size_t current_col = 0;
		
		for (size_t i = row_start; i < row_end; i++)
		{
			current_col = _col_index[i];

			if (current_col > column)
				break;

			if (current_col == column)
				return _nnz_values[i];
		}
		 
		return T();
	}
}
