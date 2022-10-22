# Vector
Шаблонный класс, реализующий последовательное хранение данных в динамической памяти. Упрощённый аналог STL - вектора. Не использует аллокаторы.

Функционал:
-------------------------

### Constructor: 

Vector() - создает пустой объект класса.

Vector(size_t) - создает вектор объектов заданного размера, инициализированных по умолчанию

Vector(const Vector&) - копирующий конструктор;

Vector(Vector&&) - перемещающий конструтоктор 


### operator=:

Vector& operator=(const Vector&) - оператор копирующего присваивания

Vector& operator=(Vector&&) - оператор перемещающего присваивания

### Iterators:

Методы возвращают итератор с указателем на начало вектора:

iterator begin()

const_iterator begin() const

const_iterator cbegin() const

Методы возвращают итератор с указателем на элемент, следующий за последним:

iterator end()

const_iterator end() const

const_iterator cend() const

### Capacity:

size_t Size() const - возвращает количество элементов вектора

size_t Capacity() const - возвращает вместимость вектора

void Reserve(size_t) - выделяет память для размещения элементов вектора

void Resize(size_t) - изменяет размер вектора

### Modifiers:

void Swap(Vector&) - обмен данными между векторами

template <typename... Args>
T& EmplaceBack(Args&&...) - создает элемент в конце вектора из набора аргументов

void PushBack(const T&) - создает элемент в конце вектора, используя копирующий конструктор

void PushBack(const T&&) - создает элемент в конце вектора, используя перемещающий конструтор

template <typename... Args>
iterator Emplace(const_iterator, Args&&...) - создает элемент в конце вектора из набора аргументов в позиции по итератору, возвращает итератор с указателем на созданный элемент

iterator Erase(const_iterator) - удаляет элемент вектора в позиции по итератору, возвращает итератор с указателем на элемент, следующий за удаленным

iterator Insert(const_iterator, const T&) - вставляет элемент вектора на позицию по итератору, использует копирующий конструктор

iterator Insert(const_iterator, T&&) - - вставляет элемент вектора на позицию по итератору, использует перемещающий конструктор

void PopBack() - удаляет последний элемент вектора 

### Element access:

const T& operator[](size_t) const - константный оператор доступа к элементу

T& operator[](size_t) - неконтактный оператор доступа к элементу  
  
-------------------------
В качетве доработки - возможно реализовать добавление стерилизованных документов в формате JSON или XML

Для сборки проекта необходим компилятор С++, поддерживающий 17 стандарт языка, или более поздние версии.
