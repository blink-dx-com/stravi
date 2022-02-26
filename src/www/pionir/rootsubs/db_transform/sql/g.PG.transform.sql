/**

  transformations for Postgres database
  @author Steffen Kube, 15.03.2019

WARNING: this time we used TABLE instead of VIEW; beacause it was not possible to 
create a VIEW with UPPER(TBALENAME) columns ...

possible views: 
	create or replace view cct_col_view as
	select c.table_name, c.column_name, c.data_type, '' comments, c.is_nullable
	from information_schema.columns c
	where table_schema = 'goz_tab';
	
	create or replace view cct_tab_view as
	select 1 as kind, t.tablename as namex, '' as comments
	FROM pg_catalog.pg_tables t where schemaname='goz_tab';

 */
 
CREATE table cct_col_view as
select upper(c.table_name) as table_name, upper(c.column_name) as column_name, c.data_type, '' comments, c.is_nullable as NULLABLE
from information_schema.columns c
where table_schema = 'goz_tab';
 
 
CREATE table cct_tab_view as
select 1 as kind, upper(t.tablename) as name, '' as comments
FROM pg_catalog.pg_tables t where schemaname='goz_tab';
 
 



