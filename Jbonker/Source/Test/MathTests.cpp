#include <jbpch.h>

#include "Jbonk/Core/utest.h"

#include "Jbonk/Math/Math.h"

using Jbonk::Math::Matrix;
using Vector = Matrix<double>;

template<typename T>
concept FloatType = std::floating_point<T>;

template<FloatType T>
struct SparseMatrixTest
{
public:
	SparseMatrixTest();
	SparseMatrixTest(const SparseMatrixTest<T>& matrix);
	SparseMatrixTest(uint16_t rows, uint16_t columns);
	SparseMatrixTest(uint16_t rows, uint16_t columns, std::vector<T> values, std::vector<size_t> col, std::vector<size_t> row);
	SparseMatrixTest(const Matrix<T>& matrix);

	inline T get(size_t row, size_t column) const;
	inline void set(size_t row, size_t column, const T& value);
	void add_value(size_t row, size_t column, const T& value);
	void update_row_indices(size_t row);

	SparseMatrixTest<T>& operator=(const SparseMatrixTest<T> matrix);

	uint16_t _rows, _columns;

	std::vector<T> _nnz_values;
	std::vector<size_t> _col_indices;
	std::vector<size_t> _row_ptr;
};

template<FloatType T>
SparseMatrixTest<T>::SparseMatrixTest()
	: _rows(0), _columns(0)
{
}

template<FloatType T>
SparseMatrixTest<T>::SparseMatrixTest(const SparseMatrixTest<T>& matrix)
{
	this->_rows = matrix._rows;
	this->_columns = matrix._columns;
	this->_nnz_values = matrix._nnz_values;
	this->_col_indices = matrix._col_indices;
	this->_row_ptr = matrix._row_ptr;
}

template<FloatType T>
SparseMatrixTest<T>::SparseMatrixTest(uint16_t rows, uint16_t columns)
	: _rows(rows), _columns(columns)
{
	_row_ptr.reserve(rows + 1);
}

template<FloatType T>
SparseMatrixTest<T>::SparseMatrixTest(uint16_t rows, uint16_t columns, std::vector<T> values, std::vector<size_t> colIndices, std::vector<size_t> rowIndices)
	:_rows(rows), _columns(columns), _nnz_values(values), _col_indices(colIndices), _row_ptr(rowIndices)
{
}

template<FloatType T>
SparseMatrixTest<T>::SparseMatrixTest(const Matrix<T>& matrix) //const Matrix<FloatType>& matrix
	: _rows(matrix.Rows()), _columns(matrix.Columns())
{
	_row_ptr.reserve(_rows + 1);
	_row_ptr.push_back(0);
	uint16_t count = 0;

	for (size_t i = 0; i < _rows; i++)
	{
		for (size_t j = 0; j < _columns; j++)
		{
			if (matrix[i][j] == T())
				continue;

			_nnz_values.push_back(matrix[i][j]);
			_col_indices.push_back(j);
			count++;
		}

		_row_ptr.push_back(count);
	}
}

template<FloatType T>
void SparseMatrixTest<T>::update_row_indices(size_t row)
{
	if (row == _rows - 1)
		_row_ptr[row + 1] += 1;

	for (size_t i = _row_ptr[row]; i < _row_ptr.size(); i++)
		_row_ptr[i] += 1;
}

template<FloatType T>
SparseMatrixTest<T>& SparseMatrixTest<T>::operator=(const SparseMatrixTest<T> matrix)
{
	this->_rows = matrix._rows;
	this->_columns = matrix._columns;
	this->_nnz_values = matrix._nnz_values;
	this->_col_indices = matrix._col_indices;
	this->_row_ptr = matrix._row_ptr;

	return *this;
}

template<FloatType T>
void SparseMatrixTest<T>::add_value(size_t row, size_t column, const T& value)
{
	JB_CORE_ASSERT(row < _rows || column < _columns);

	if (value == T())
		return;

	if (_nnz_values.empty())
	{
		_nnz_values.push_back(value);
		_col_indices.push_back(column);
		_row_ptr[0] = 0;
		return;
	}

	size_t row_start = _row_ptr[row];
	size_t row_end = _row_ptr[row + 1] - 1;

	if (_col_indices[row_start] == column)
	{
		_nnz_values[row_start] = value;
		return;
	}

	if (_col_indices[row_end] < column)
	{
		_nnz_values.insert(_nnz_values.begin() + row_end + 1, value);
		_col_indices.insert(_col_indices.begin() + row_end + 1, column);
		update_row_indices(row);
		return;
	}

	if (_col_indices[row_start] > column)
	{
		_nnz_values.insert(_nnz_values.begin() + row_start, value);
		_col_indices.insert(_col_indices.begin() + row_start, column);
		update_row_indices(row);
		return;
	}

	for (size_t i = _col_indices[row_start]; i < _col_indices[row_end] + 1; i++)
	{
		if (i == column)
		{
			_nnz_values.insert(_nnz_values.begin() + i + 2, value);
			_col_indices.insert(_col_indices.begin() + i + 2, column);
			update_row_indices(row);
			return;
		}
	}
}

template<FloatType T>
T SparseMatrixTest<T>::get(size_t row, size_t column) const
{
	JB_CORE_ASSERT(row < _rows || column < _columns);

	size_t row_start = _row_ptr[row];
	size_t row_end = _row_ptr[row + 1];
	size_t current_col = 0;

	for (size_t i = row_start; i < row_end; i++)
	{
		current_col = _col_indices[i];

		if (current_col > column)
			break;

		if (current_col == column)
			return _nnz_values[i];
	}

	return T();
}

template<FloatType T>
void SparseMatrixTest<T>::set(size_t row, size_t column, const T& value)
{
	//if (value == T()) consider deleting the entry if value is 0.0

	JB_CORE_ASSERT(row < _rows || column < _columns);
	JB_CORE_ASSERT(_nnz_values.size());
	size_t row_start = _row_ptr[row];
	size_t row_end = _row_ptr[row + 1];
	size_t current_col = 0;

	for (size_t i = row_start; i < row_end; i++)
	{
		current_col = _col_indices[i];

		if (current_col == column)
			_nnz_values[i] = value;

		if (current_col > column)
			break;
	}
}

template<FloatType T>
static Vector sparse_csr_matrix_vector(const SparseMatrixTest<T>& A, const Vector& x)
{
	Vector res = Matrix<double>(A._rows, 1);

	for (size_t row = 0; row < A._rows; row++)
	{
		T r0 = T();

		for (size_t i = A._row_ptr[row]; i < A._row_ptr[row + 1]; i++)
			r0 += A._nnz_values[i] * x[A._col_indices[i]][0];

		res[row][0] = r0;
	}

	return res;
}

template<FloatType T>
static SparseMatrixTest<T> sparse_csr_matrix_matrix(const SparseMatrixTest<T>& A, const SparseMatrixTest<T>& B)
{
	JB_CORE_ASSERT(A._rows == B._rows);
	JB_CORE_ASSERT(A._columns == B._columns);

	SparseMatrixTest<T> res(A._rows, A._rows);

	size_t count = 0;
	res._row_ptr.push_back(count);

	for (size_t i = 0; i < A._rows; i++)
	{
		for (size_t j = 0; j < B._rows; j++)
		{
			T r0 = T();

			for (size_t k = A._row_ptr[i]; k < A._row_ptr[i + 1]; k++)
			{
				r0 += A._nnz_values[k] * B.get(j, A._col_indices[k]);
			}

			if (r0 == T())
				continue;

			res._nnz_values.push_back(r0);
			res._col_indices.push_back(j);
			count++;
		}

		res._row_ptr.push_back(count);
	}

	return res;
}

template<FloatType T>
static SparseMatrixTest<T> sparse_csr_transpose(const SparseMatrixTest<T>& A)
{
	SparseMatrixTest<T> res(
		A._columns,
		A._rows,
		std::vector<T>(A._nnz_values.size(), 0.0),
		std::vector<size_t>(A._col_indices.size(), 0),
		std::vector<size_t>(A._columns + 2, 0)
	);

	//count nnz per column
	for (size_t i = 0; i < A._nnz_values.size(); i++)
		++res._row_ptr[A._col_indices[i] + 2];

	//generate shifted i ptr
	for (size_t i = 2; i < res._row_ptr.size(); i++)
		res._row_ptr[i] += res._row_ptr[i - 1];

	for (size_t row = 0; row < A._rows; row++)
	{
		for (size_t i = A._row_ptr[row]; i < A._row_ptr[row + 1]; i++)
		{
			const size_t index = res._row_ptr[A._col_indices[i] + 1]++;
			res._nnz_values[index] = A._nnz_values[i];
			res._col_indices[index] = row;
		}
	}

	res._row_ptr.pop_back();

	return res;
}

template<FloatType T>
static SparseMatrixTest<T> sparse_csr_transpose_dl(const SparseMatrixTest<T>& A, const Vector& dl)
{
	JB_CORE_ASSERT(A._columns == dl.Rows());

	SparseMatrixTest<T> res(
		A._columns,
		A._rows,
		std::vector<T>(A._nnz_values.size(), 0.0),
		std::vector<size_t>(A._col_indices.size(), 0),
		std::vector<size_t>(A._columns + 2, 0)
	);

	for (size_t i = 0; i < A._nnz_values.size(); i++)
		++res._row_ptr[A._col_indices[i] + 2];

	for (size_t i = 2; i < res._row_ptr.size(); i++)
		res._row_ptr[i] += res._row_ptr[i - 1];

	for (size_t row = 0; row < A._rows; row++)
	{
		for (size_t i = A._row_ptr[row]; i < A._row_ptr[row + 1]; i++)
		{
			const size_t index = res._row_ptr[A._col_indices[i] + 1]++;
			res._nnz_values[index] = A._nnz_values[i] * dl[A._col_indices[i]][0];
			res._col_indices[index] = row;
		}
	}

	res._row_ptr.pop_back();

	return res;
}

template<FloatType T>
static SparseMatrixTest<T> sparse_csr_dl(const SparseMatrixTest<T>& A, const Vector& dl)
{
	JB_CORE_ASSERT(A._columns == dl.Rows());

	SparseMatrixTest<T> res = A;

	for (size_t row = 0; row < A._rows; row++)
	{
		for (size_t i = A._row_ptr[row]; i < A._row_ptr[row + 1]; i++)
		{
			res._nnz_values[i] *= dl[A._col_indices[i]][0];
		}
	}

	return res;
}

static Vector sparse_csr_matrix_vector_f(const SparseMatrixTest<double>& A, const Vector& x) //this method works well for sparse matrices with dense rows
{
	Vector res = Matrix<double>(A._rows, 1);

	for (size_t row = 0; row < A._rows; row++)
	{
		size_t row_start = A._row_ptr[row];
		size_t row_end = A._row_ptr[row + 1];
		size_t len = row_end - row_start;
		size_t residual_index = 0;
		double residual = 0.0;
		__m256d _sum, _x_qword, _A_qword, _cbc_prod, _sum_sh, _lane_sh;
		_sum = _mm256_setzero_pd();

		for (size_t i = row_start; i < row_end; i += 4)
		{
			if ((i - row_start) + 4 > len)
			{
				residual_index = i - row_start;
				break;
			}

			_x_qword = _mm256_set_pd(x[A._col_indices[i + 3]][0], x[A._col_indices[i + 2]][0], x[A._col_indices[i + 1]][0], x[A._col_indices[i]][0]);
			_A_qword = _mm256_loadu_pd(&A._nnz_values[i]);
			_cbc_prod = _mm256_mul_pd(_x_qword, _A_qword);
			_sum = _mm256_add_pd(_sum, _cbc_prod);
		}

		_sum_sh = _mm256_shuffle_pd(_sum, _sum, _MM_SHUFFLE(0, 0, 1, 1));
		_sum = _mm256_add_pd(_sum, _sum_sh);
		_lane_sh = _mm256_permute2f128_pd(_sum, _sum, _MM_SHUFFLE(0, 2, 0, 1));
		_sum = _mm256_add_pd(_sum, _lane_sh);
		res[row][0] = _mm256_cvtsd_f64(_sum);

		if (len % 4 == 0)
			continue;
		
		if (len < 4)
		{
			for (size_t i = row_start; i < row_end; i++)
				residual += A._nnz_values[i] * x[A._col_indices[i]][0];
		}
		else
		{
			for (size_t i = row_start + residual_index; i < row_end; i++)
				residual += A._nnz_values[i] * x[A._col_indices[i]][0];
		}

		res[row][0] += residual;
	}

	return res;
}

struct SparseMatrixFixture
{
	Matrix<double> matrix;
	Matrix<double> dense_matrix;
	Vector vector;
	SparseMatrixTest<double> sparseMatrix;
};

UTEST_F_SETUP(SparseMatrixFixture)
{
	utest_fixture->matrix = Matrix<double>(4, 6);
	utest_fixture->dense_matrix = Matrix<double>(3, 7);
	utest_fixture->vector = Matrix<double>(6, 1);

	utest_fixture->matrix[0][0] = 10.0;
	utest_fixture->matrix[0][1] = 20.0;
	utest_fixture->matrix[1][1] = 30.0;
	utest_fixture->matrix[1][3] = 40.0;
	utest_fixture->matrix[2][2] = 50.0;
	utest_fixture->matrix[2][3] = 60.0;
	utest_fixture->matrix[2][4] = 70.0;
	utest_fixture->matrix[3][5] = 80.0;

	utest_fixture->dense_matrix(0) = 1.0;
	utest_fixture->dense_matrix(1) = 2.0;
	utest_fixture->dense_matrix(2) = 3.0;
	utest_fixture->dense_matrix(3) = 4.0;
	utest_fixture->dense_matrix(4) = 5.0;
	utest_fixture->dense_matrix(5) = 6.0;
	utest_fixture->dense_matrix(6) = 7.0;
	utest_fixture->dense_matrix(7) = 8.0;
	utest_fixture->dense_matrix(8) = 9.0;
	utest_fixture->dense_matrix(9) = 10.0;
	utest_fixture->dense_matrix(10) = 11.0;
	utest_fixture->dense_matrix(11) = 12.0;
	utest_fixture->dense_matrix(12) = 13.0;
	utest_fixture->dense_matrix(13) = 14.0;
	utest_fixture->dense_matrix(14) = 15.0;
	utest_fixture->dense_matrix(15) = 16.0;
	utest_fixture->dense_matrix(16) = 17.0;
	utest_fixture->dense_matrix(17) = 18.0;
	utest_fixture->dense_matrix(18) = 19.0;
	utest_fixture->dense_matrix(19) = 20.0;
	utest_fixture->dense_matrix(20) = 21.0;

	utest_fixture->vector[0][0] = 4.0;
	utest_fixture->vector[1][0] = 6.0;
	utest_fixture->vector[2][0] = 10.0;
	utest_fixture->vector[3][0] = 20.0;
	utest_fixture->vector[4][0] = 22.0;
	utest_fixture->vector[5][0] = 16.0;

	utest_fixture->sparseMatrix = SparseMatrixTest<double>(utest_fixture->matrix);

	ASSERT_EQ(utest_fixture->dense_matrix.Size(), 21);
}

UTEST_F_TEARDOWN(SparseMatrixFixture)
{
	

}

UTEST_F(SparseMatrixFixture, MatrixIterator)
{
	Matrix<double> matrix_t(3, 7);
	uint32_t index = 0;

	for (Matrix<double>::iterator it = utest_fixture->dense_matrix.begin(); it != utest_fixture->dense_matrix.end(); it++, index++)
	{
		matrix_t(index) = (*it);
	}

	ASSERT_EQ(utest_fixture->dense_matrix[0][0], matrix_t(0));
	ASSERT_EQ(utest_fixture->dense_matrix[0][1], matrix_t(1));
	ASSERT_EQ(utest_fixture->dense_matrix[0][2], matrix_t(2));
	ASSERT_EQ(utest_fixture->dense_matrix[0][3], matrix_t(3));
	ASSERT_EQ(utest_fixture->dense_matrix[0][4], matrix_t(4));
	ASSERT_EQ(utest_fixture->dense_matrix[0][5], matrix_t(5));
	ASSERT_EQ(utest_fixture->dense_matrix[0][6], matrix_t(6));
	ASSERT_EQ(utest_fixture->dense_matrix[1][0], matrix_t(7));
	ASSERT_EQ(utest_fixture->dense_matrix[1][1], matrix_t(8));
	ASSERT_EQ(utest_fixture->dense_matrix[1][2], matrix_t(9));
	ASSERT_EQ(utest_fixture->dense_matrix[1][3], matrix_t(10));
	ASSERT_EQ(utest_fixture->dense_matrix[1][4], matrix_t(11));
	ASSERT_EQ(utest_fixture->dense_matrix[1][5], matrix_t(12));
	ASSERT_EQ(utest_fixture->dense_matrix[1][6], matrix_t(13));
	ASSERT_EQ(utest_fixture->dense_matrix[2][0], matrix_t(14));
	ASSERT_EQ(utest_fixture->dense_matrix[2][1], matrix_t(15));
	ASSERT_EQ(utest_fixture->dense_matrix[2][2], matrix_t(16));
	ASSERT_EQ(utest_fixture->dense_matrix[2][3], matrix_t(17));
	ASSERT_EQ(utest_fixture->dense_matrix[2][4], matrix_t(18));
	ASSERT_EQ(utest_fixture->dense_matrix[2][5], matrix_t(19));
	ASSERT_EQ(utest_fixture->dense_matrix[2][6], matrix_t(20));
}

UTEST_F(SparseMatrixFixture, MatrixConstruction)
{
	ASSERT_EQ(utest_fixture->sparseMatrix._nnz_values[0], 10.0);
	ASSERT_EQ(utest_fixture->sparseMatrix._nnz_values[1], 20.0);
	ASSERT_EQ(utest_fixture->sparseMatrix._nnz_values[2], 30.0);
	ASSERT_EQ(utest_fixture->sparseMatrix._nnz_values[3], 40.0);
	ASSERT_EQ(utest_fixture->sparseMatrix._nnz_values[4], 50.0);
	ASSERT_EQ(utest_fixture->sparseMatrix._nnz_values[5], 60.0);
	ASSERT_EQ(utest_fixture->sparseMatrix._nnz_values[6], 70.0);
	ASSERT_EQ(utest_fixture->sparseMatrix._nnz_values[7], 80.0);

	ASSERT_EQ(utest_fixture->sparseMatrix._col_indices[0], 0);
	ASSERT_EQ(utest_fixture->sparseMatrix._col_indices[1], 1);
	ASSERT_EQ(utest_fixture->sparseMatrix._col_indices[2], 1);
	ASSERT_EQ(utest_fixture->sparseMatrix._col_indices[3], 3);
	ASSERT_EQ(utest_fixture->sparseMatrix._col_indices[4], 2);
	ASSERT_EQ(utest_fixture->sparseMatrix._col_indices[5], 3);
	ASSERT_EQ(utest_fixture->sparseMatrix._col_indices[6], 4);
	ASSERT_EQ(utest_fixture->sparseMatrix._col_indices[7], 5);
		
	ASSERT_EQ(utest_fixture->sparseMatrix._row_ptr[0], 0);
	ASSERT_EQ(utest_fixture->sparseMatrix._row_ptr[1], 2);
	ASSERT_EQ(utest_fixture->sparseMatrix._row_ptr[2], 4);
	ASSERT_EQ(utest_fixture->sparseMatrix._row_ptr[3], 7);
	ASSERT_EQ(utest_fixture->sparseMatrix._row_ptr[4], 8);
			
	ASSERT_EQ(utest_fixture->sparseMatrix.get(0, 0), 10.0);
	ASSERT_EQ(utest_fixture->sparseMatrix.get(0, 1), 20.0);
	ASSERT_EQ(utest_fixture->sparseMatrix.get(1, 1), 30.0);
	ASSERT_EQ(utest_fixture->sparseMatrix.get(1, 3), 40.0);
	ASSERT_EQ(utest_fixture->sparseMatrix.get(2, 2), 50.0);
	ASSERT_EQ(utest_fixture->sparseMatrix.get(2, 3), 60.0);
	ASSERT_EQ(utest_fixture->sparseMatrix.get(2, 4), 70.0);
	ASSERT_EQ(utest_fixture->sparseMatrix.get(3, 5), 80.0);
}

UTEST_F(SparseMatrixFixture, MatrixTimesMatrix)
{
	SparseMatrixTest<double> product = sparse_csr_matrix_matrix(utest_fixture->sparseMatrix, utest_fixture->sparseMatrix);

	ASSERT_EQ(product.get(0, 0), 500.0);
	ASSERT_EQ(product.get(0, 1), 600.0);
	ASSERT_EQ(product.get(1, 0), 600.0);
	ASSERT_EQ(product.get(1, 1), 2500.0);
	ASSERT_EQ(product.get(1, 2), 2400.0);
	ASSERT_EQ(product.get(2, 1), 2400.0);
	ASSERT_EQ(product.get(2, 2), 11000.0);
	ASSERT_EQ(product.get(3, 3), 6400.0);

	ASSERT_EQ(product._nnz_values[0], 500.0);
	ASSERT_EQ(product._nnz_values[1], 600.0);
	ASSERT_EQ(product._nnz_values[2], 600.0);
	ASSERT_EQ(product._nnz_values[3], 2500.0);
	ASSERT_EQ(product._nnz_values[4], 2400.0);
	ASSERT_EQ(product._nnz_values[5], 2400.0);
	ASSERT_EQ(product._nnz_values[6], 11000.0);
	ASSERT_EQ(product._nnz_values[7], 6400.0);

	ASSERT_EQ(product._col_indices[0], 0);
	ASSERT_EQ(product._col_indices[1], 1);
	ASSERT_EQ(product._col_indices[2], 0);
	ASSERT_EQ(product._col_indices[3], 1);
	ASSERT_EQ(product._col_indices[4], 2);
	ASSERT_EQ(product._col_indices[5], 1);
	ASSERT_EQ(product._col_indices[6], 2);
	ASSERT_EQ(product._col_indices[7], 3);

	ASSERT_EQ(product._row_ptr[0], 0);
	ASSERT_EQ(product._row_ptr[1], 2);
	ASSERT_EQ(product._row_ptr[2], 5);
	ASSERT_EQ(product._row_ptr[3], 7);
	ASSERT_EQ(product._row_ptr[4], 8);
}

UTEST_F(SparseMatrixFixture, MatrixTranspose)
{
	SparseMatrixTest<double> transpose = sparse_csr_transpose(utest_fixture->sparseMatrix);

	ASSERT_EQ(transpose.get(0, 0), 10.0);
	ASSERT_EQ(transpose.get(1, 0), 20.0);
	ASSERT_EQ(transpose.get(1, 1), 30.0);
	ASSERT_EQ(transpose.get(2, 2), 50.0);
	ASSERT_EQ(transpose.get(3, 1), 40.0);
	ASSERT_EQ(transpose.get(3, 2), 60.0);
	ASSERT_EQ(transpose.get(4, 2), 70.0);
	ASSERT_EQ(transpose.get(5, 3), 80.0);

	ASSERT_EQ(transpose._nnz_values[0], 10.0);
	ASSERT_EQ(transpose._nnz_values[1], 20.0);
	ASSERT_EQ(transpose._nnz_values[2], 30.0);
	ASSERT_EQ(transpose._nnz_values[3], 50.0);
	ASSERT_EQ(transpose._nnz_values[4], 40.0);
	ASSERT_EQ(transpose._nnz_values[5], 60.0);
	ASSERT_EQ(transpose._nnz_values[6], 70.0);
	ASSERT_EQ(transpose._nnz_values[7], 80.0);

	ASSERT_EQ(transpose._col_indices[0], 0);
	ASSERT_EQ(transpose._col_indices[1], 0);
	ASSERT_EQ(transpose._col_indices[2], 1);
	ASSERT_EQ(transpose._col_indices[3], 2);
	ASSERT_EQ(transpose._col_indices[4], 1);
	ASSERT_EQ(transpose._col_indices[5], 2);
	ASSERT_EQ(transpose._col_indices[6], 2);
	ASSERT_EQ(transpose._col_indices[7], 3);

	ASSERT_EQ(transpose._row_ptr[0], 0);
	ASSERT_EQ(transpose._row_ptr[1], 1);
	ASSERT_EQ(transpose._row_ptr[2], 3);
	ASSERT_EQ(transpose._row_ptr[3], 4);
	ASSERT_EQ(transpose._row_ptr[4], 6);
	ASSERT_EQ(transpose._row_ptr[5], 7);
	ASSERT_EQ(transpose._row_ptr[6], 8);

}

UTEST_F(SparseMatrixFixture, diagonalLeftTranspose)
{
	SparseMatrixTest<double> scaled = sparse_csr_transpose_dl(utest_fixture->sparseMatrix, utest_fixture->vector);

	ASSERT_EQ(scaled.get(0, 0), 40.0);
	ASSERT_EQ(scaled.get(1, 0), 120.0);
	ASSERT_EQ(scaled.get(1, 1), 180.0);
	ASSERT_EQ(scaled.get(2, 2), 500.0);
	ASSERT_EQ(scaled.get(3, 1), 800.0);
	ASSERT_EQ(scaled.get(3, 2), 1200.0);
	ASSERT_EQ(scaled.get(4, 2), 1540.0);
	ASSERT_EQ(scaled.get(5, 3), 1280.0);
}

UTEST_F(SparseMatrixFixture, diagonalLeft)
{
	SparseMatrixTest<double> scaled = sparse_csr_dl(utest_fixture->sparseMatrix, utest_fixture->vector);

	ASSERT_EQ(scaled.get(0, 0), 40.0);
	ASSERT_EQ(scaled.get(0, 1), 120.0);
	ASSERT_EQ(scaled.get(1, 1), 180.0);
	ASSERT_EQ(scaled.get(2, 2), 500.0);
	ASSERT_EQ(scaled.get(1, 3), 800.0);
	ASSERT_EQ(scaled.get(2, 3), 1200.0);
	ASSERT_EQ(scaled.get(2, 4), 1540.0);
	ASSERT_EQ(scaled.get(3, 5), 1280.0);
}

UTEST_F(SparseMatrixFixture, changeValues)
{
	utest_fixture->sparseMatrix.set(2, 2, 75.0);
	utest_fixture->sparseMatrix.set(3, 5, 120.0);
	utest_fixture->sparseMatrix.set(0, 1, 400.0);

	ASSERT_EQ(utest_fixture->sparseMatrix._row_ptr[0], 0);
	ASSERT_EQ(utest_fixture->sparseMatrix._row_ptr[1], 2);
	ASSERT_EQ(utest_fixture->sparseMatrix._row_ptr[2], 4);
	ASSERT_EQ(utest_fixture->sparseMatrix._row_ptr[3], 7);
	ASSERT_EQ(utest_fixture->sparseMatrix._row_ptr[4], 8);

	ASSERT_EQ(utest_fixture->sparseMatrix.get(2, 2), 75.0);
	ASSERT_EQ(utest_fixture->sparseMatrix.get(3, 5), 120.0);
	ASSERT_EQ(utest_fixture->sparseMatrix.get(0, 1), 400.0);
}

UTEST(SparseMatrixTest, MatrixIndex)
{
	constexpr size_t row = 4;
	constexpr size_t col = 6;

	SparseMatrixTest<double> sparseMatrix = SparseMatrixTest<double>(row, col);
	sparseMatrix._row_ptr.clear();
	sparseMatrix._nnz_values.push_back(10.0);
	sparseMatrix._nnz_values.push_back(20.0);
	sparseMatrix._nnz_values.push_back(30.0);
	sparseMatrix._nnz_values.push_back(40.0);
	sparseMatrix._nnz_values.push_back(50.0);
	sparseMatrix._nnz_values.push_back(60.0);
	sparseMatrix._nnz_values.push_back(70.0);
	sparseMatrix._nnz_values.push_back(80.0);

	sparseMatrix._col_indices.push_back(0);
	sparseMatrix._col_indices.push_back(1);
	sparseMatrix._col_indices.push_back(1);
	sparseMatrix._col_indices.push_back(3);
	sparseMatrix._col_indices.push_back(2);
	sparseMatrix._col_indices.push_back(3);
	sparseMatrix._col_indices.push_back(4);
	sparseMatrix._col_indices.push_back(5);

	sparseMatrix._row_ptr.push_back(0);
	sparseMatrix._row_ptr.push_back(2);
	sparseMatrix._row_ptr.push_back(4);
	sparseMatrix._row_ptr.push_back(7);
	sparseMatrix._row_ptr.push_back(8);

	ASSERT_EQ(sparseMatrix.get(0, 0), 10.0);
	ASSERT_EQ(sparseMatrix.get(0, 1), 20.0);
	ASSERT_EQ(sparseMatrix.get(1, 1), 30.0);
	ASSERT_EQ(sparseMatrix.get(1, 3), 40.0);
	ASSERT_EQ(sparseMatrix.get(2, 2), 50.0);
	ASSERT_EQ(sparseMatrix.get(2, 3), 60.0);
	ASSERT_EQ(sparseMatrix.get(2, 4), 70.0);
	ASSERT_EQ(sparseMatrix.get(3, 5), 80.0);
	ASSERT_EQ(sparseMatrix.get(2, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 2), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 3), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 4), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 5), 0.0);
	ASSERT_EQ(sparseMatrix.get(1, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(1, 2), 0.0);
	ASSERT_EQ(sparseMatrix.get(1, 4), 0.0);
	ASSERT_EQ(sparseMatrix.get(1, 5), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 1), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 5), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 1), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 2), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 3), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 4), 0.0);

	sparseMatrix.add_value(3, 1, 25.0);
	sparseMatrix.add_value(1, 0, 15.0);
	sparseMatrix.add_value(1, 5, 95.0);
	sparseMatrix.add_value(1, 2, 45.0);

	ASSERT_EQ(sparseMatrix._row_ptr[0], 0);
	ASSERT_EQ(sparseMatrix._row_ptr[1], 2);
	ASSERT_EQ(sparseMatrix._row_ptr[2], 7);
	ASSERT_EQ(sparseMatrix._row_ptr[3], 10);
	ASSERT_EQ(sparseMatrix._row_ptr[4], 12);

	ASSERT_EQ(sparseMatrix._nnz_values[0], 10.0);
	ASSERT_EQ(sparseMatrix._nnz_values[1], 20.0);
	ASSERT_EQ(sparseMatrix._nnz_values[2], 15.0);
	ASSERT_EQ(sparseMatrix._nnz_values[3], 30.0);
	ASSERT_EQ(sparseMatrix._nnz_values[4], 45.0);
	ASSERT_EQ(sparseMatrix._nnz_values[5], 40.0);
	ASSERT_EQ(sparseMatrix._nnz_values[6], 95.0);
	ASSERT_EQ(sparseMatrix._nnz_values[7], 50.0);
	ASSERT_EQ(sparseMatrix._nnz_values[8], 60.0);
	ASSERT_EQ(sparseMatrix._nnz_values[9], 70.0);
	ASSERT_EQ(sparseMatrix._nnz_values[10], 25.0);
	ASSERT_EQ(sparseMatrix._nnz_values[11], 80.0);

	ASSERT_EQ(sparseMatrix.get(0, 0), 10.0);
	ASSERT_EQ(sparseMatrix.get(0, 1), 20.0);
	ASSERT_EQ(sparseMatrix.get(1, 1), 30.0);
	ASSERT_EQ(sparseMatrix.get(1, 3), 40.0);
	ASSERT_EQ(sparseMatrix.get(2, 2), 50.0);
	ASSERT_EQ(sparseMatrix.get(2, 3), 60.0);
	ASSERT_EQ(sparseMatrix.get(2, 4), 70.0);
	ASSERT_EQ(sparseMatrix.get(3, 5), 80.0);
	ASSERT_EQ(sparseMatrix.get(1, 0), 15.0);
	ASSERT_EQ(sparseMatrix.get(1, 2), 45.0);
	ASSERT_EQ(sparseMatrix.get(1, 5), 95.0);
	ASSERT_EQ(sparseMatrix.get(3, 1), 25.0);
	ASSERT_EQ(sparseMatrix.get(2, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 2), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 3), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 4), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 5), 0.0);
	ASSERT_EQ(sparseMatrix.get(1, 4), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 1), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 5), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 2), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 3), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 4), 0.0);
}

UTEST(SparseMatrixTest, SparseTimesVector)
{
	constexpr size_t row = 4;
	constexpr size_t col = 6;

	SparseMatrixTest<double> sparseMatrix = SparseMatrixTest<double>(row, col);
	sparseMatrix._row_ptr.clear();
	sparseMatrix._nnz_values.push_back(10.0);
	sparseMatrix._nnz_values.push_back(20.0);
	sparseMatrix._nnz_values.push_back(30.0);
	sparseMatrix._nnz_values.push_back(40.0);
	sparseMatrix._nnz_values.push_back(50.0);
	sparseMatrix._nnz_values.push_back(60.0);
	sparseMatrix._nnz_values.push_back(70.0);
	sparseMatrix._nnz_values.push_back(80.0);

	sparseMatrix._col_indices.push_back(0);
	sparseMatrix._col_indices.push_back(1);
	sparseMatrix._col_indices.push_back(1);
	sparseMatrix._col_indices.push_back(3);
	sparseMatrix._col_indices.push_back(2);
	sparseMatrix._col_indices.push_back(3);
	sparseMatrix._col_indices.push_back(4);
	sparseMatrix._col_indices.push_back(5);

	sparseMatrix._row_ptr.push_back(0);
	sparseMatrix._row_ptr.push_back(2);
	sparseMatrix._row_ptr.push_back(4);
	sparseMatrix._row_ptr.push_back(7);
	sparseMatrix._row_ptr.push_back(8);

	Vector x = Matrix<double>(col, 1);
	x[0][0] = 2.0;
	x[1][0] = 6.0;
	x[2][0] = 9.0;
	x[3][0] = 3.0;
	x[4][0] = 1.0;
	x[5][0] = 8.0;

	Vector b = Matrix<double>(row, 1); 
	b = sparse_csr_matrix_vector(sparseMatrix, x);

	ASSERT_EQ(b[0][0], 140.0);
	ASSERT_EQ(b[1][0], 300.0);
	ASSERT_EQ(b[2][0], 700.0);
	ASSERT_EQ(b[3][0], 640.0);
}

UTEST(SparseMatrixTest, fastVectorMul)
{
	SparseMatrixTest<double> sparseMatrix = SparseMatrixTest<double>(8, 12);
	Vector x(12, 1);
	x[0][0] = 1.0;
	x[1][0] = 2.0;
	x[2][0] = 3.0;
	x[3][0] = 4.0;
	x[4][0] = 5.0;
	x[5][0] = 6.0;
	x[6][0] = 7.0;
	x[7][0] = 8.0;
	x[8][0] = 9.0;
	x[9][0] = 10.0;
	x[10][0] = 11.0;
	x[11][0] = 12.0;

	double V[35] = { 2.0, 4.0, 1.0, 10.0, 2.0, 3.0, 4.0, 12.0, 2.0, 1.0, 1.0, 4.0, 4.0, 4.0, 2.0, 1.0, 9.0, 1.0, 10.0,
		3.0, 2.0, 9.0, 1.0, 1.0, 1.0, 12.0, 1.0, 4.0, 7.0, 4.0, 2.0, 9.0, 2.0, 3.0, 1.0 };
	size_t C[35] = { 0, 1, 2, 4, 7, 9, 10, 11, 1, 2, 6, 7, 2, 5, 8, 9, 2, 4, 9, 3, 4, 7, 2, 3, 4, 7, 10, 2, 6, 8, 10, 3, 4, 7, 11 };
	size_t R[9] = { 0, 8, 12, 16, 19, 22, 27, 31, 35 };

	for (size_t i = 0; i < 35; i++)
	{
		sparseMatrix._nnz_values.push_back(V[i]);
		sparseMatrix._col_indices.push_back(C[i]);
	}

	sparseMatrix._row_ptr.clear();

	for (size_t i = 0; i < 9; i++)
	{
		sparseMatrix._row_ptr.push_back(R[i]);
	}

	Vector result = sparse_csr_matrix_vector_f(sparseMatrix, x);

	ASSERT_EQ(result[0][0], 297.0);
	ASSERT_EQ(result[1][0], 46.0);
	ASSERT_EQ(result[2][0], 64.0);
	ASSERT_EQ(result[3][0], 132.0);
	ASSERT_EQ(result[4][0], 94.0);
	ASSERT_EQ(result[5][0], 119.0);
	ASSERT_EQ(result[6][0], 119.0);
	ASSERT_EQ(result[7][0], 82.0);
}

UTEST(Intrinsics, example)
{
	constexpr size_t row = 4;
	constexpr size_t col = 6;

	SparseMatrixTest<double> sparseMatrix = SparseMatrixTest<double>(row, col);
	sparseMatrix._row_ptr.clear();
	sparseMatrix._nnz_values.push_back(2.0);
	sparseMatrix._nnz_values.push_back(4.0);
	sparseMatrix._nnz_values.push_back(1.0);
	sparseMatrix._nnz_values.push_back(10.0);
	sparseMatrix._nnz_values.push_back(2.0);
	sparseMatrix._nnz_values.push_back(3.0);
	sparseMatrix._nnz_values.push_back(4.0);
	sparseMatrix._nnz_values.push_back(12.0);

	sparseMatrix._col_indices.push_back(0);
	sparseMatrix._col_indices.push_back(1);
	sparseMatrix._col_indices.push_back(2);
	sparseMatrix._col_indices.push_back(4);
	sparseMatrix._col_indices.push_back(7);
	sparseMatrix._col_indices.push_back(9);
	sparseMatrix._col_indices.push_back(10);
	sparseMatrix._col_indices.push_back(11);

	sparseMatrix._row_ptr.push_back(0);
	sparseMatrix._row_ptr.push_back(8);
	sparseMatrix._row_ptr.push_back(12);
	sparseMatrix._row_ptr.push_back(16);
	sparseMatrix._row_ptr.push_back(19);

	Vector vec1 = Matrix<double>(16, 1);
	Vector vec2 = Matrix<double>(16, 1);

	vec1[0][0] = 1.0;
	vec1[1][0] = 2.0;
	vec1[2][0] = 3.0;
	vec1[3][0] = 4.0;
	vec1[4][0] = 5.0;
	vec1[5][0] = 6.0;
	vec1[6][0] = 7.0;
	vec1[7][0] = 8.0;
	vec1[8][0] = 9.0;
	vec1[9][0] = 10.0;
	vec1[10][0] = 11.0;
	vec1[11][0] = 12.0;
	vec1[12][0] = 13.0;
	vec1[13][0] = 14.0;
	vec1[14][0] = 15.0;
	vec1[15][0] = 16.0;

	std::vector<double> values;
	values.push_back(1.0);
	values.push_back(2.0);
	values.push_back(3.0);
	values.push_back(4.0);
	values.push_back(5.0);
	values.push_back(6.0);
	values.push_back(7.0);
	values.push_back(8.0);

	Vector x = Matrix<double>(6, 1);

	__m256d _num1, _num2, _num3, _num4, _num5, _num6, _num7, _num8, _num9;
	_num1 = _mm256_loadu_pd(&vec1[0][0]);
	_num2 = _mm256_loadu_pd(&values[0]);
	_num3 = _mm256_loadu_pd(&vec1[4][0]);
	_num4 = _mm256_loadu_pd(&values[4]);
	//_num9 = _mm256_loadu_pd(&ref_values[0]);

	_num5 = _mm256_mul_pd(_num1, _num2);
	_num6 = _mm256_add_pd(_num3, _num4);
	_num7 = _mm256_fmadd_pd(_num1, _num2, _num1);

	const int shuf1  _MM_SHUFFLE(1, 1, 1, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 4.0, 3.0] W
	const int shuf2  _MM_SHUFFLE(2, 3, 1, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 4.0, 3.0] W
	const int shuf3  _MM_SHUFFLE(2, 3, 1, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 4.0, 3.0] L
	const int shuf4  _MM_SHUFFLE(2, 3, 0, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 3.0] L
	const int shuf5  _MM_SHUFFLE(2, 3, 0, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 3.0] L
	const int shuf6  _MM_SHUFFLE(1, 0, 0, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 3.0] L
	const int shuf7  _MM_SHUFFLE(1, 1, 0, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 3.0] L
	const int shuf8  _MM_SHUFFLE(1, 1, 1, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 4.0, 3.0] L
	const int shuf9  _MM_SHUFFLE(1, 0, 1, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 4.0, 3.0] L
	const int shuf10 _MM_SHUFFLE(0, 0, 1, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 4.0, 3.0] L
	const int shuf11 _MM_SHUFFLE(0, 1, 0, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 3.0] L
	const int shuf12 _MM_SHUFFLE(1, 0, 0, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 3.0] L
	const int shuf13 _MM_SHUFFLE(0, 0, 0, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 3.0] L
	const int shuf14 _MM_SHUFFLE(0, 0, 1, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 4.0, 3.0] W
	const int shuf15 _MM_SHUFFLE(0, 1, 1, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 4.0, 3.0] L
	const int shuf16 _MM_SHUFFLE(0, 2, 0, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 3.0] L
	const int shuf17 _MM_SHUFFLE(2, 0, 0, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 3.0] L
	const int shuf19 _MM_SHUFFLE(0, 0, 2, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 4.0] L
	const int shuf20 _MM_SHUFFLE(0, 0, 1, 2); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 2.0, 4.0, 3.0] L
	const int shuf21 _MM_SHUFFLE(0, 0, 2, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 4.0] L
	const int shuf22 _MM_SHUFFLE(0, 0, 2, 3); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 2.0, 3.0, 4.0] L
	const int shuf18 _MM_SHUFFLE2(1, 1);      //[1.0, 2.0, 3.0, 4.0] -> [2.0, 2.0, 3.0, 3.0] L
	const int shuf23 _MM_SHUFFLE2(0, 1);      //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 3.0] L
	const int shuf24 _MM_SHUFFLE2(1, 0);      //[1.0, 2.0, 3.0, 4.0] -> [2.0, 2.0, 3.0, 3.0] L
	const int shuf25 _MM_SHUFFLE2(1, 2);      //[1.0, 2.0, 3.0, 4.0] -> [1.0, 2.0, 3.0, 3.0] L
	_num8 = _mm256_permute_pd(_num1, shuf2); 
	_num2 = _mm256_shuffle_pd(_num1, _num1, shuf2);
	__m256d n1 = _mm256_shuffle_pd(_num1, _num1, shuf19);
	__m256d n2 = _mm256_shuffle_pd(_num1, _num1, shuf23);
	__m256d n3 = _mm256_shuffle_pd(_num1, _num1, shuf14);
	__m256d n4 = _mm256_permute2f128_pd(n3, n3, 0x21);
	double testNum = _mm256_cvtsd_f64(n4);

	_mm256_storeu_pd(&vec2[0][0], _num5);
	_mm256_storeu_pd(&vec2[4][0], _num6);
	_mm256_storeu_pd(&vec2[8][0], _num7);
	_mm256_storeu_pd(&vec2[12][0], _num8);
	//_mm256_storeu_pd(&vec1[0][0], _num9);

	size_t rowIndex = 0;
	__m256d _sum, _vec, _mat_part, _cbc_prod, _sumhs, _lane;
	_sum = _mm256_setzero_pd();
	size_t n = sparseMatrix._row_ptr[rowIndex + 1] - sparseMatrix._row_ptr[rowIndex];
	for (size_t i = sparseMatrix._row_ptr[rowIndex]; i < n - n % 4; i += 4)
	{
		_vec = _mm256_set_pd(vec1[sparseMatrix._col_indices[i + 3]][0], vec1[sparseMatrix._col_indices[i + 2]][0],
			vec1[sparseMatrix._col_indices[i + 1]][0], vec1[sparseMatrix._col_indices[i]][0]);

		_mat_part = _mm256_loadu_pd(&sparseMatrix._nnz_values[i]);
		_cbc_prod = _mm256_mul_pd(_vec, _mat_part);
		_sum = _mm256_add_pd(_sum, _cbc_prod);
	}
	_sumhs = _mm256_shuffle_pd(_sum, _sum, _MM_SHUFFLE(0, 0, 1, 1));
	_sum = _mm256_add_pd(_sum, _sumhs);
	_lane = _mm256_permute2f128_pd(_sum, _sum, _MM_SHUFFLE(0, 2, 0, 1));
	_sum = _mm256_add_pd(_lane, _sum);
	double sum = _mm256_cvtsd_f64(_sum);

	ASSERT_EQ(vec2[0][0], 1.0);
	ASSERT_EQ(vec2[1][0], 4.0);
	ASSERT_EQ(vec2[2][0], 9.0);
	ASSERT_EQ(vec2[3][0], 16.0);
	ASSERT_EQ(vec2[4][0], 10.0);
	ASSERT_EQ(vec2[5][0], 12.0);
	ASSERT_EQ(vec2[6][0], 14.0);
	ASSERT_EQ(vec2[7][0], 16.0);
	ASSERT_EQ(vec2[8][0], 2.0);
	ASSERT_EQ(vec2[9][0], 6.0);
	ASSERT_EQ(vec2[10][0], 12.0);
	ASSERT_EQ(vec2[11][0], 20.0);
	ASSERT_EQ(vec2[12][0], 2.0);
	ASSERT_EQ(vec2[13][0], 1.0);
	ASSERT_EQ(vec2[14][0], 4.0);
	ASSERT_EQ(vec2[15][0], 3.0);

	ASSERT_EQ(vec1[0][0], 1.0);
	ASSERT_EQ(vec1[1][0], 2.0);
	ASSERT_EQ(vec1[2][0], 3.0);
	ASSERT_EQ(vec1[3][0], 4.0);
}

