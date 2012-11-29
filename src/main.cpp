#include <iostream>
#include <string>

#include <sql/sql.hpp>
#include <web/web.hpp>
#include <render/render.hpp>

/**
 * Structure of "task" table.
 */
struct task: table
{
	field<int> id;
	field<std::string> description;

	task()
		: table("task")
		, id(this, "id")
		, description(this, "description")
	{
	}
};

/**
 * Context of index template.
 */
struct index_context
{
	int total_tasks;
};

/*
 * Index template
 */
static auto index_tpl = "<html>"
	"<head>"
	"<title>Example C++ web app</title>"
	"</head>"
	"<body>"
	"Total tasks: " + render::get(&index_context::total_tasks) + "."
	"</body>"
	"</html>";

int
main(int argc, char * argv[])
{
	try
	{
		// Database connection.
		database db("sqlite://todo.sqlite3");
		// Creating table
		db.session().create_table<task>();
		// Setting up web application server.
		web::application app(argc, argv);
		
		// Show main page with information about tasks count.
		app.get("/", [&db](web::request &, web::response & res) {	
			// Setup template context
			index_context ctx;
			ctx.total_tasks = db.session().query<task>().count();
			// Setup scope.
			render::scope s;
			s.push(ctx);
			// Render template
			res.stream() << index_tpl(s);
		});
		std::cout << "Application is running..." << std::endl;
		app.listen(3333);
	}
	catch (std::exception e)
	{
		std::cerr << "Caught exception: " << e.what() << std::endl;
		return 1;
	}
}