/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2018 Michael Drake <tlsa@netsurf-browser.org>
 */

#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <cyaml.h>

#include "ttest.h"

typedef struct test_data {
	cyaml_data_t **data;
	unsigned *seq_count;
	const struct cyaml_config *config;
	const struct cyaml_schema_type *schema;
} test_data_t;

/* Common cleanup function to free data loaded by tests. */
static void cyaml_cleanup(void *data)
{
	struct test_data *td = data;
	unsigned seq_count = 0;

	if (td->seq_count != NULL) {
		seq_count = *(td->seq_count);
	}

	cyaml_free(td->config, td->schema, *(td->data), seq_count);
}

/* Test loading a non-existent file. */
static bool test_file_load_bad_path(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	struct target_struct {
		char *cakes;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_file("/cyaml/path/shouldn't/exist.yaml",
			config, &top_schema, (cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_FILE_OPEN) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	return ttest_pass(&tc);
}

/* Test loading the basic YAML file. */
static bool test_file_load_basic(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	struct animal {
		char *kind;
		char **sounds;
		unsigned sounds_count;
	};
	struct target_struct {
		struct animal *animals;
		unsigned animals_count;
		char **cakes;
		unsigned cakes_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type sounds_entry_schema = {
		CYAML_TYPE_STRING(CYAML_FLAG_POINTER, char, 0, CYAML_UNLIMITED),
	};
	static const struct cyaml_schema_mapping animal_mapping_schema[] = {
		CYAML_MAPPING_STRING_PTR("kind", CYAML_FLAG_POINTER,
				struct animal, kind, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_SEQUENCE("sounds", CYAML_FLAG_POINTER,
				struct animal, sounds,
				&sounds_entry_schema, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type animals_entry_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_DEFAULT,
				struct animal, animal_mapping_schema),
	};
	static const struct cyaml_schema_type cakes_entry_schema = {
		CYAML_TYPE_STRING(CYAML_FLAG_POINTER, char, 0, CYAML_UNLIMITED),
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_SEQUENCE("animals", CYAML_FLAG_POINTER,
				struct target_struct, animals,
				&animals_entry_schema, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_SEQUENCE("cakes", CYAML_FLAG_POINTER,
				struct target_struct, cakes,
				&cakes_entry_schema, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_file("test/data/basic.yaml", config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_OK) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	return ttest_pass(&tc);
}

/* Test loading the basic YAML file, with a mismatching schema. */
static bool test_file_load_basic_invalid(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	struct animal {
		char *kind;
		int *sounds;
		unsigned sounds_count;
	};
	struct target_struct {
		struct animal *animals;
		unsigned animals_count;
		char **cakes;
		unsigned cakes_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type sounds_entry_schema = {
		/* The data has a string, but we're expecting int here. */
		CYAML_TYPE_INT(CYAML_FLAG_DEFAULT, int),
	};
	static const struct cyaml_schema_mapping animal_mapping_schema[] = {
		CYAML_MAPPING_STRING_PTR("kind", CYAML_FLAG_POINTER,
				struct animal, kind, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_SEQUENCE("sounds", CYAML_FLAG_POINTER,
				struct animal, sounds,
				&sounds_entry_schema, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type animals_entry_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_DEFAULT,
				struct animal, animal_mapping_schema),
	};
	static const struct cyaml_schema_type cakes_entry_schema = {
		CYAML_TYPE_STRING(CYAML_FLAG_POINTER, char, 0, CYAML_UNLIMITED),
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_SEQUENCE("animals", CYAML_FLAG_POINTER,
				struct target_struct, animals,
				&animals_entry_schema, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_SEQUENCE("cakes", CYAML_FLAG_POINTER,
				struct target_struct, cakes,
				&cakes_entry_schema, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_file("test/data/basic.yaml", config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	return ttest_pass(&tc);
}

/**
 * Run the YAML file tests.
 *
 * \param[in]  rc         The ttest report context.
 * \param[in]  log_level  CYAML log level.
 * \param[in]  log_fn     CYAML logging function, or NULL.
 * \return true iff all unit tests pass, otherwise false.
 */
bool file_tests(
		ttest_report_ctx_t *rc,
		cyaml_log_t log_level,
		cyaml_log_fn_t log_fn)
{
	bool pass = true;
	cyaml_config_t config = {
		.log_fn = log_fn,
		.mem_fn = cyaml_mem,
		.log_level = log_level,
		.flags = CYAML_CFG_DEFAULT,
	};

	ttest_heading(rc, "File loading tests");

	pass &= test_file_load_basic(rc, &config);

	/* Since we expect loads of error logging for these tests,
	 * suppress log output if required log level is greater
	 * than \ref CYAML_LOG_INFO.
	 */
	if (log_level > CYAML_LOG_INFO) {
		config.log_fn = NULL;
	}

	pass &= test_file_load_bad_path(rc, &config);
	pass &= test_file_load_basic_invalid(rc, &config);

	return pass;
}
