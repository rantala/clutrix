clutrix: clutrix.c
	gcc -Wall -Wextra -g -O2 `pkg-config clutter-1.0 --cflags --libs` $< -o $@

clean:
	rm -f clutrix
