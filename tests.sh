#!/bin/bash

./pipe --print_tokens --print_ast --print_ir ./examples/images/01-rgb_to_grayscale.pipe
./pipe --print_tokens --print_ast --print_ir ./examples/images/02-brightness_adjustment.pipe
./pipe --print_tokens --print_ast --print_ir ./examples/images/03-contrast_adjustment.pipe
./pipe --print_tokens --print_ast --print_ir ./examples/images/04-gamma_correction.pipe
./pipe --print_tokens --print_ast --print_ir ./examples/images/05-binarization.pipe
