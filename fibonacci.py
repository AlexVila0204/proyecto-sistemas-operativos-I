
def fibonacci(n):
    a, b = 0, 1
    for _ in range(n):
        a, b = b, a + b
    return a

if __name__ == "__main__":
    n = int(input("Ingrese un número: "))
    print(f"El número {n} en la secuencia de Fibonacci es: {fibonacci(n)}")
