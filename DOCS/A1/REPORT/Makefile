# Makefile for 

# Target file
TARGET = 21CS10016_PART_A_REPORT.pdf 21CS10016_PART_B_REPORT.pdf

# LaTeX compiler
LATEX = pdflatex

# Default target
all: $(TARGET)

# Compile LaTeX document
$(TARGET): %.pdf: %.tex
	$(LATEX) $<
	$(LATEX) $<

# Clean up
clean:
	rm -f *.aux *.log *.out *.toc *.bcf *.xml