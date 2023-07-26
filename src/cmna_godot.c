#include <string.h>

#include <gdnative_api_struct.gen.h>

#include <cmna/cmna.h>

const godot_gdnative_core_api_struct *api = NULL;
const godot_gdnative_ext_nativescript_api_struct *native_api = NULL;
const godot_gdnative_ext_nativescript_1_1_api_struct *native_1_1_api = NULL;

typedef struct {
	struct cmna_circuit circuit;
	int is_init;
} circuit_data;

void *godot_mem_calloc(size_t n, size_t sz) {
	void *region = api->godot_alloc(n*sz);
	memset(region, 0, n*sz);
	return region;
}

void godot_mem_free(void *region) {
	api->godot_free(region);
}

struct cmna_alloc GODOT_ALLOC = {
	.calloc = godot_mem_calloc,
	.free = godot_mem_free,
};

void *circuit_constructor(godot_object *inst, void *method_data);
void circuit_destructor(godot_object *inst, void *method_data, void *user_data);

#define METHOD_ARGS godot_object *inst, void *method_data, void *user_data, int argc, godot_variant **argv
#define METHOD_FN(nm) godot_variant circuit_method_##nm(METHOD_ARGS)
METHOD_FN(init);
METHOD_FN(strerror);
METHOD_FN(solve);
METHOD_FN(add_conductance);
METHOD_FN(add_current);
METHOD_FN(add_source_terms);
METHOD_FN(add_source_potential);

#define GETTER_ARGS godot_object *inst, void *method_data, void *user_data
#define GETTER_FN(nm) godot_variant circuit_get_##nm(GETTER_ARGS)
GETTER_FN(nodes);
GETTER_FN(sources);
GETTER_FN(potentials);
GETTER_FN(currents);

void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *options) {
	api = options->api_struct;

	for(int i = 0; i < api->num_extensions; i++) {
		switch(api->extensions[i]->type) {
			case GDNATIVE_EXT_NATIVESCRIPT:
				native_api = (godot_gdnative_ext_nativescript_api_struct *)api->extensions[i];
				const godot_gdnative_api_struct *n = native_api->next;
				if(!n) break;
				if(n->version.major == 1 && n->version.minor == 1) {
					native_1_1_api = (godot_gdnative_ext_nativescript_1_1_api_struct *)n;
				}
				break;
		}
	}
	if(!(native_api && native_1_1_api)) {
		api->godot_print_error("FATAL: cmna-godot couldn't find the right extensions! Your engine is about to crash!", __FUNCTION__, __FILE__, __LINE__);
	}
}
void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *options) {
	api = NULL;
	native_api = NULL;
	native_1_1_api = NULL;
}

void GDN_EXPORT godot_nativescript_init(void *handle) {
	godot_instance_create_func circuit_create = {
		circuit_constructor, NULL, NULL,
	};
	godot_instance_destroy_func circuit_destroy = {
		circuit_destructor, NULL, NULL,
	};

	godot_method_attributes no_rpc = { GODOT_METHOD_RPC_MODE_DISABLED };

	native_api->godot_nativescript_register_class(handle, "Circuit", "Reference", circuit_create, circuit_destroy);

	{
		godot_string doc;
		api->godot_string_new(&doc);
		api->godot_string_parse_utf8(&doc, "A libcmna circuit.\n\
\n\
You MUST initialize this before use, using .init(nodes, sources). Then, \
use the add_* methods to set up the circuit, and use .solve() to compute \
a solution, which can be gotten via the properties .potentials (one per \
node) and .currents (one per source).\n\
\n\
At any time, you can call .init() again to create a fresh circuit state, \
removing any previous contributions or state. Note that this will reallocate, \
so avoid it if possible. Individual methods will mention how to reverse \
the contributions of one component.");
		native_1_1_api->godot_nativescript_set_class_documentation(handle, "Circuit", doc);
		api->godot_string_destroy(&doc);
	}

#define METHOD(nm) { \
	godot_instance_method meth = {circuit_method_##nm, NULL, NULL}; \
	native_api->godot_nativescript_register_method(handle, "Circuit", #nm, no_rpc, meth); \
}

	METHOD(init);
	METHOD(strerror);
	METHOD(solve);
	METHOD(add_conductance);
	METHOD(add_current);
	METHOD(add_source_terms);
	METHOD(add_source_potential);

#define _MP_DOC(kind, nm, s) { \
	godot_string doc; \
	api->godot_string_new(&doc); \
	api->godot_string_parse_utf8(&doc, s); \
	native_1_1_api->godot_nativescript_set_##kind##_documentation(handle, "Circuit", #nm, doc); \
	api->godot_string_destroy(&doc); \
}
#define METHOD_DOC(nm, s) _MP_DOC(method, nm, s)
#define PROP_DOC(nm, s) _MP_DOC(property, nm, s)
#define MAKE_ARG(tp, nm) godot_method_arg nm = {\
	.type = GODOT_VARIANT_TYPE_##tp, \
	.hint = GODOT_PROPERTY_HINT_NONE, \
}; \
api->godot_string_new(&nm.name); \
api->godot_string_parse_utf8(&nm.name, #nm); \
api->godot_string_new(&nm.hint_string)
#define CLEAN_ARG(nm) api->godot_string_destroy(&nm.name); \
api->godot_string_destroy(&nm.hint_string)
#define ARG_INFO(nm, cnt, ...) native_1_1_api->godot_nativescript_set_method_argument_information(handle, "Circuit", #nm, cnt, (godot_method_arg[]){ __VA_ARGS__ })

	METHOD_DOC(init, "Initialize the circuit. Must be called before anything else.");
	{
		MAKE_ARG(INT, nodes);
		MAKE_ARG(INT, sources);
		ARG_INFO(init, 2, nodes, sources);
		CLEAN_ARG(nodes); CLEAN_ARG(sources);
	}
	METHOD_DOC(strerror, "Turn a status code into a human-readable string.");
	{
		MAKE_ARG(INT, status);
		ARG_INFO(strerror, 1, status);
		CLEAN_ARG(status);
	}
	METHOD_DOC(solve, "Solve the circuit, returning a a status.");
	METHOD_DOC(add_conductance, "Add a conductance between two nodes (with null being ground), returning a status.\n\
\n\
To reverse: add a negative conductance to the same nodes.");
	{
		MAKE_ARG(INT, node_a);
		MAKE_ARG(INT, node_b);
		MAKE_ARG(REAL, conductance);
		ARG_INFO(add_conductance, 3, node_a, node_b, conductance);
		CLEAN_ARG(node_a); CLEAN_ARG(node_b); CLEAN_ARG(conductance);
	}
	METHOD_DOC(add_current, "Add a current at a node, returning a status.\n\
\n\
To reverse: add a negative current at the same node.");
	{
		MAKE_ARG(INT, node);
		MAKE_ARG(REAL, current);
		ARG_INFO(add_current, 2, node, current);
		CLEAN_ARG(node); CLEAN_ARG(current);
	}
	METHOD_DOC(add_source_terms, "Connect a voltage source to nodes (possibly null = ground), returning a status.\n\
\n\
To reverse: connect the same source with the terminal nodes swapped.");
	{
		MAKE_ARG(INT, source);
		MAKE_ARG(INT, node_positive);
		MAKE_ARG(INT, node_negative);
		ARG_INFO(add_source_terms, 3, source, node_positive, node_negative);
		CLEAN_ARG(source); CLEAN_ARG(node_positive); CLEAN_ARG(node_negative);
	}
	METHOD_DOC(add_source_potential, "Add potential to a voltage source, returning a status.\n\
\n\
To reverse: add negative potential to the same source.");
	{
		MAKE_ARG(INT, source);
		MAKE_ARG(REAL, potential);
		ARG_INFO(add_source_potential, 2, source, potential);
		CLEAN_ARG(source); CLEAN_ARG(potential);
	}

#define GETTER(tp, cons, nm, dfl, decl, init, destr) { \
	godot_property_attributes attr = { \
		.rset_type = GODOT_METHOD_RPC_MODE_DISABLED, \
		.type = GODOT_VARIANT_TYPE_##tp, \
		.hint = GODOT_PROPERTY_HINT_NONE, \
		.usage = GODOT_PROPERTY_USAGE_NOEDITOR, \
	}; \
	godot_property_get_func getter = {circuit_get_##nm, NULL, NULL}; \
	godot_property_set_func setter = {NULL, NULL, NULL}; \
	decl; \
	init; \
	api->godot_string_new(&attr.hint_string); \
	api->godot_variant_new_##cons(&attr.default_value, dfl); \
	native_api->godot_nativescript_register_property(handle, "Circuit", #nm, &attr, setter, getter); \
	api->godot_variant_destroy(&attr.default_value); \
	api->godot_string_destroy(&attr.hint_string); \
	destr; \
}

	GETTER(INT, int, nodes, -1,,,);
	GETTER(INT, int, sources, -1,,,);
	GETTER(POOL_REAL_ARRAY, pool_real_array, potentials, &empty, godot_pool_real_array empty, api->godot_pool_real_array_new(&empty), api->godot_pool_real_array_destroy(&empty));
	GETTER(POOL_REAL_ARRAY, pool_real_array, currents, &empty, godot_pool_real_array empty, api->godot_pool_real_array_new(&empty), api->godot_pool_real_array_destroy(&empty));

	PROP_DOC(nodes, "Get the number of nodes in the circuit. Prior to .init(), this value is null.");
	PROP_DOC(sources, "Get the number of sources in the circuit. Prior to .init(), this value is null.");
	PROP_DOC(potentials, "If the circuit is successfully solved, evaluates to a \
PoolRealArray containing the voltage at every node, as indexed by node \
number.\n\
\n\
If the circuit isn't initialized at all, this will return null. If the \
circuit hasn't been solved, or some other error occurs, this returns the \
error code (probably CMNA_E_NOT_READY).");
	PROP_DOC(currents, "If the circuit is successfully solved, evaluates to a \
PoolRealArray containing the currents at every source, as indexed by source \
number.\n\
\n\
If the circuit isn't initialized at all, this will return null. If the \
ircuit hasn't been solved, or some other error occurs, this returns the \
error code (probably CMNA_E_NOT_READY).");
}

void *circuit_constructor(godot_object *inst, void *method_data) {
	circuit_data *data = api->godot_alloc(sizeof(circuit_data));
	data->is_init = 0;
}

void circuit_destructor(godot_object *inst, void *method_data, void *user_data) {
	circuit_data *data = (circuit_data *)user_data;
	if(data->is_init) {
		cmna_circuit_cleanup(&data->circuit);
	}
}

#define PREAMBLE circuit_data *data = (circuit_data *)user_data; \
	godot_variant ret; \
	api->godot_variant_new_nil(&ret)

#define ARGCK(n) if(argc != n) { \
	api->godot_print_error("Wrong number of arguments (expected " #n ")", __FUNCTION__, __FILE__, __LINE__); \
	return ret; \
}

METHOD_FN(init) {
	PREAMBLE;
	ARGCK(2);

	uint64_t nodes = api->godot_variant_as_uint(argv[0]);
	uint64_t sources = api->godot_variant_as_uint(argv[1]);

	if(data->is_init) {
		cmna_circuit_cleanup(&data->circuit);
	}
	api->godot_variant_destroy(&ret);
	enum cmna_error err = cmna_circuit_init(&data->circuit, CMNA_PREC_SINGLE, nodes, sources, &GODOT_ALLOC);
	api->godot_variant_new_int(&ret, (int64_t) err);
	if(err == CMNA_E_SUCCESS) data->is_init = 1;
	return ret;
}

METHOD_FN(strerror) {
	PREAMBLE;
	godot_string str;
	ARGCK(1);

	api->godot_variant_destroy(&ret);
	api->godot_string_new(&str);
	api->godot_string_parse_utf8(&str, cmna_strerror((enum cmna_error)api->godot_variant_as_int(argv[0])));
	api->godot_variant_new_string(&ret, &str);
	api->godot_string_destroy(&str);
	return ret;
}

METHOD_FN(solve) {
	PREAMBLE;
	ARGCK(0);
	enum cmna_error error = CMNA_E_NOT_READY;
	if(data->is_init) {
		error = cmna_circuit_solve(&data->circuit);
	}
	api->godot_variant_destroy(&ret);
	api->godot_variant_new_int(&ret, (int64_t) error);
	return ret;
}

METHOD_FN(add_conductance) {
	PREAMBLE;
	ARGCK(3);
	godot_variant_type ta = api->godot_variant_get_type(argv[0]);
	godot_variant_type tb = api->godot_variant_get_type(argv[1]);
	double cond = api->godot_variant_as_real(argv[2]);
	enum cmna_error error = CMNA_E_SUCCESS;
	api->godot_variant_destroy(&ret);

	if(!data->is_init) {
		api->godot_variant_new_int(&ret, (int64_t)CMNA_E_NOT_READY);
		return ret;
	}

	if(ta == GODOT_VARIANT_TYPE_NIL && tb == GODOT_VARIANT_TYPE_NIL) {
		/* Nothing to do */
	} else if(ta == GODOT_VARIANT_TYPE_NIL) {
		error = cmna_circuit_add_conductance_to_ground(&data->circuit, (size_t)api->godot_variant_as_uint(argv[1]), cond);
	} else if(tb == GODOT_VARIANT_TYPE_NIL) {
		error = cmna_circuit_add_conductance_to_ground(&data->circuit, (size_t)api->godot_variant_as_uint(argv[0]), cond);
	} else {
		error = cmna_circuit_add_conductance(&data->circuit,
				(size_t)api->godot_variant_as_uint(argv[0]),
				(size_t)api->godot_variant_as_uint(argv[1]),
				cond
		);
	}
	api->godot_variant_new_int(&ret, (int64_t) error);
	return ret;
}

METHOD_FN(add_current) {
	PREAMBLE;
	ARGCK(2);
	api->godot_variant_destroy(&ret);
	enum cmna_error error = CMNA_E_NOT_READY;

	if(data->is_init) {
		error = cmna_circuit_add_current(&data->circuit,
				(size_t)api->godot_variant_as_uint(argv[0]),
				api->godot_variant_as_real(argv[1])
		);
	}

	api->godot_variant_new_int(&ret, (int64_t)error);
	return ret;
}

METHOD_FN(add_source_terms) {
	PREAMBLE;
	ARGCK(3);
	api->godot_variant_destroy(&ret);
	size_t src = (size_t)api->godot_variant_as_uint(argv[0]);
	enum cmna_error error = CMNA_E_NOT_READY;

	if(data->is_init) {
		error = CMNA_E_SUCCESS;
		if(api->godot_variant_get_type(argv[1]) != GODOT_VARIANT_TYPE_NIL) {
			error = cmna_circuit_add_source_pos(&data->circuit,
					src, (size_t)api->godot_variant_as_uint(argv[1])
			);
		}
		if(error == CMNA_E_SUCCESS && api->godot_variant_get_type(argv[2]) != GODOT_VARIANT_TYPE_NIL) {
			error = cmna_circuit_add_source_neg(&data->circuit,
					src, (size_t)api->godot_variant_as_uint(argv[2])
			);
		}
	}
	
	api->godot_variant_new_int(&ret, (int64_t)error);
	return ret;
}

METHOD_FN(add_source_potential) {
	PREAMBLE;
	ARGCK(2);
	api->godot_variant_destroy(&ret);
	enum cmna_error error = CMNA_E_NOT_READY;

	if(data->is_init) {
		error = cmna_circuit_add_source_potential(&data->circuit,
				(size_t)api->godot_variant_as_uint(argv[0]),
				api->godot_variant_as_real(argv[1])
		);
	}

	api->godot_variant_new_int(&ret, (int64_t)error);
	return ret;
}

GETTER_FN(nodes) {
	PREAMBLE;
	if(data->is_init) {
		api->godot_variant_destroy(&ret);
		api->godot_variant_new_uint(&ret, data->circuit.nodes);
	}
	return ret;
}

GETTER_FN(sources) {
	PREAMBLE;
	if(data->is_init) {
		api->godot_variant_destroy(&ret);
		api->godot_variant_new_uint(&ret, data->circuit.sources);
	}
	return ret;
}

GETTER_FN(potentials) {
	PREAMBLE;
	enum cmna_error error;
	if(!data->is_init) return ret;
	api->godot_variant_destroy(&ret);

	float *vtg;

	if((error = cmna_circuit_node_potentials(&data->circuit, (void **)&vtg))) {
		api->godot_variant_new_int(&ret, (int64_t) error);
		return ret;
	}

	godot_pool_real_array pra;
	api->godot_pool_real_array_new(&pra);
	api->godot_pool_real_array_resize(&pra, data->circuit.nodes);
	godot_pool_array_write_access *access = api->godot_pool_real_array_write(&pra);
	godot_real *arr = api->godot_pool_real_array_write_access_ptr(access);
	memcpy(arr, vtg, sizeof(godot_real) * data->circuit.nodes);
	api->godot_pool_real_array_write_access_destroy(access);
	api->godot_variant_new_pool_real_array(&ret, &pra);
	api->godot_pool_real_array_destroy(&pra);
	return ret;
}

GETTER_FN(currents) {
	PREAMBLE;
	enum cmna_error error;
	if(!data->is_init) return ret;
	api->godot_variant_destroy(&ret);

	float *cur;

	if((error = cmna_circuit_source_currents(&data->circuit, (void **)&cur))) {
		api->godot_variant_new_int(&ret, (int64_t) error);
		return ret;
	}

	godot_pool_real_array pra;
	api->godot_pool_real_array_new(&pra);
	api->godot_pool_real_array_resize(&pra, data->circuit.sources);
	godot_pool_array_write_access *access = api->godot_pool_real_array_write(&pra);
	godot_real *arr = api->godot_pool_real_array_write_access_ptr(access);
	memcpy(arr, cur, sizeof(godot_real) * data->circuit.sources);
	api->godot_pool_real_array_write_access_destroy(access);
	api->godot_variant_new_pool_real_array(&ret, &pra);
	api->godot_pool_real_array_destroy(&pra);
	return ret;
}
