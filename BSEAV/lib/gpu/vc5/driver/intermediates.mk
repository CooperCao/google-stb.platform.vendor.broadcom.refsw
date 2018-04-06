define generated_src_dir_exists
$(hide)mkdir -p $(GLSL_INTERMEDIATE_ABS_PATH)
$(hide)mkdir -p $(DGLENUM_INTERMEDIATE_ABS_PATH)
endef

glsl_primitive_types_deps := \
	$(V3D_DRIVER_LIBS_REL_PATH)/khrn/glsl/scripts/build_primitive_types.py \
	$(V3D_DRIVER_LIBS_REL_PATH)/khrn/glsl/scripts/scalar_types.table \
	$(V3D_DRIVER_LIBS_REL_PATH)/khrn/glsl/scripts/image_types.table

define glsl_primitive_types_gen
$(generated_src_dir_exists)
$(hide) \
	$(PYTHON_CMD) \
		$(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/scripts/build_primitive_types.py \
		-I $(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl \
		-O $(GLSL_INTERMEDIATE_ABS_PATH) \
		$(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/scripts/scalar_types.table \
		$(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/scripts/image_types.table;
endef

$(GLSL_INTERMEDIATE_ABS_PATH)/glsl_primitive_types.auto.table : \
	$(glsl_primitive_types_deps)
	$(glsl_primitive_types_gen)

$(GLSL_INTERMEDIATE_REL_PATH)/glsl_primitive_type_index.auto.h \
$(GLSL_INTERMEDIATE_REL_PATH)/glsl_primitive_types.auto.h \
$(GLSL_INTERMEDIATE_REL_PATH)/glsl_primitive_types.auto.c : \
	$(GLSL_INTERMEDIATE_ABS_PATH)/glsl_primitive_types.auto.table
		@

$(GLSL_INTERMEDIATE_REL_PATH)/glsl_intrinsic_lookup.auto.h : \
	$(V3D_DRIVER_LIBS_REL_PATH)/khrn/glsl/glsl_intrinsic_lookup.gperf
	$(generated_src_dir_exists)
	$(hide)$(GPERF_CMD) $(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/glsl_intrinsic_lookup.gperf > $(GLSL_INTERMEDIATE_ABS_PATH)/glsl_intrinsic_lookup.auto.h

$(GLSL_INTERMEDIATE_REL_PATH)/glsl_layout.auto.h : \
	$(V3D_DRIVER_LIBS_REL_PATH)/khrn/glsl/glsl_layout.gperf
	$(generated_src_dir_exists)
	$(hide)$(GPERF_CMD) $(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/glsl_layout.gperf > $(GLSL_INTERMEDIATE_ABS_PATH)/glsl_layout.auto.h

define textures_auto_gen
$(generated_src_dir_exists)
$(hide) \
	$(PYTHON_CMD) \
	$(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/scripts/build_texture_functions.py \
	-O $(GLSL_INTERMEDIATE_ABS_PATH);
endef

$(GLSL_INTERMEDIATE_ABS_PATH)/textures.auto.props : \
	$(V3D_DRIVER_LIBS_REL_PATH)/khrn/glsl/scripts/build_texture_functions.py
	$(textures_auto_gen)

$(GLSL_INTERMEDIATE_ABS_PATH)/textures.auto.glsl : \
	$(GLSL_INTERMEDIATE_ABS_PATH)/textures.auto.props
		@

define glsl_stdlib_auto_gen
$(generated_src_dir_exists)
$(hide)$(PYTHON_CMD) \
	$(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/scripts/build_stdlib.py \
	-I $(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl \
	-O $(GLSL_INTERMEDIATE_ABS_PATH) \
	$(STDLIB_SOURCES) \
	$(STDLIB_AUTO_SOURCES)
endef

$(GLSL_INTERMEDIATE_REL_PATH)/glsl_stdlib.auto.c : \
	$(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/scripts/build_stdlib.py \
	$(addprefix $(V3D_DRIVER_LIBS_REL_PATH)/khrn/glsl/,$(STDLIB_SOURCES)) $(STDLIB_AUTO_SOURCES)
		$(glsl_stdlib_auto_gen)

$(GLSL_INTERMEDIATE_REL_PATH)/glsl_stdlib.auto.h : \
$(GLSL_INTERMEDIATE_REL_PATH)/glsl_stdlib.auto.c
	@

$(GLSL_INTERMEDIATE_REL_PATH)/glsl_parser.output : \
	$(GLSL_INTERMEDIATE_REL_PATH)/glsl_layout.auto.h \
	$(GLSL_INTERMEDIATE_REL_PATH)/glsl_intrinsic_lookup.auto.h \
	$(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/glsl_parser.y
		$(generated_src_dir_exists)
		$(hide)$(BISON_CMD) -d -o $(GLSL_INTERMEDIATE_ABS_PATH)/glsl_parser.c $(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/glsl_parser.y

$(GLSL_INTERMEDIATE_REL_PATH)/glsl_parser.c \
$(GLSL_INTERMEDIATE_REL_PATH)/glsl_parser.h : \
$(GLSL_INTERMEDIATE_REL_PATH)/glsl_parser.output
	@

$(GLSL_INTERMEDIATE_REL_PATH)/glsl_lexer.c : \
	$(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/glsl_lexer.l
	$(generated_src_dir_exists)
	$(hide)$(FLEX_CMD) -L -o $(GLSL_INTERMEDIATE_ABS_PATH)/glsl_lexer.c --never-interactive $(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/glsl_lexer.l

$(GLSL_INTERMEDIATE_REL_PATH)/glsl_numbers.c : \
	$(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/glsl_numbers.l
	$(generated_src_dir_exists)
	$(hide)$(FLEX_CMD) -L -o $(GLSL_INTERMEDIATE_ABS_PATH)/glsl_numbers.c --never-interactive $(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/glsl_numbers.l

$(GLSL_INTERMEDIATE_REL_PATH)/glsl_version.c : \
	$(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/glsl_version.l
	$(generated_src_dir_exists)
	$(hide)$(FLEX_CMD) -L -o $(GLSL_INTERMEDIATE_ABS_PATH)/glsl_version.c --never-interactive $(V3D_DRIVER_LIBS_ABS_PATH)/khrn/glsl/glsl_version.l

$(DGLENUM_INTERMEDIATE_REL_PATH)/dglenum_gen.h : \
	$(V3D_DRIVER_LIBS_REL_PATH)/util/dglenum/dglenum_gen.py \
	$(V3D_DRIVER_LIBS_REL_PATH)/khrn/include/GLES3/gl3.h \
	$(V3D_DRIVER_LIBS_REL_PATH)/khrn/include/GLES3/gl3ext_brcm.h
	$(generated_src_dir_exists)
	$(PYTHON_CMD) $(V3D_DRIVER_LIBS_ABS_PATH)/util/dglenum/dglenum_gen.py $(V3D_DRIVER_TOP_ABS_PATH)/driver > $(DGLENUM_INTERMEDIATE_ABS_PATH)/dglenum_gen.h

$(V3D_DRIVER_TOP_REL_PATH)/driver/libs/util/dglenum/dglenum.c : $(DGLENUM_INTERMEDIATE_REL_PATH)/dglenum_gen.h
