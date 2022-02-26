/* reset all sequences to 1, variable: DBUSER_TAB will be replaced */

CREATE OR REPLACE FUNCTION sequences_reset () 
   RETURNS INTEGER AS $$ 
DECLARE
   seq_name text; 
   query text;
   alter_query text;
   alter_query2 text;
   rec RECORD;
BEGIN
   query := 'select sequence_name from information_schema.sequences where sequence_schema=''DBUSER_TAB'' order by sequence_name';
   FOR rec IN EXECUTE query LOOP
      alter_query := 'ALTER SEQUENCE  ' || rec.sequence_name || ' RESTART WITH 1'; 
      alter_query2 := ' SELECT setval( ''' || rec.sequence_name || ''', 1)'; 
      RAISE NOTICE '- %', alter_query;
      /*EXECUTE alter_query; */
      EXECUTE alter_query;
      EXECUTE alter_query2;
      
   END LOOP;
   
   RAISE NOTICE 'READY';
   return 1;
END ; 
$$ LANGUAGE plpgsql;

SELECT "sequences_reset"();