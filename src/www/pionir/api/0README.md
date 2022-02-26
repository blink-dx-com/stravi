# Interfaces to the world; e.g. JSONRPC, REST, 

## JSONRPC - Interface

### Login

Call: DEF/login

### Typical calls to/from Blinkbox

#### load Assay 

Browse projects in DB
- Call: DEF/oPROJ_getObjects('id:, 'tables':, 'getpath':)
- Call: LAB/oABSTRACT_PROTO_load('id')

#### load Experiment 

Browse projects in DB
- Call: DEF/oPROJ_getObjects('id:, 'tables':, 'getpath':)

Load experiment
- Call: LAB/oEXP_load( 'id':, 'g_doc_list':0,1 )

Download documents: ... call N-times ...
- Call: LAB/oEXP_datafile( 'id', 'filepath')

#### save Experiment 

Browse projects in DB
- Call: DEF/oPROJ_getObjects('id:, 'tables':, 'getpath':)

Create experiment
- Call: LAB/oEXP_create( 'args':, 'json': )

Upload documents:
- REST-Call: set_doc( file:, filename:,  parameters:)