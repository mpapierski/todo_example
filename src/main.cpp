#include <iostream>
#include <string>
#include <list>
#include <cstdlib>

#include <sql/sql.hpp>
#include <web/web.hpp>
#include <render/render.hpp>

// Database config

static std::string const database_url = []() -> const char * {
	char * env = ::getenv("DATABASE_URL");
	if (env)
	{
		return env;
	}
	return "sqlite://todo.sqlite3";
}();

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

	std::string get_description()
	{
		return this->description;
	}
};

/**
 * Header context
 */
struct header_context
{
	std::string title;
};

/**
 * Context of index template.
 */
struct index_context
{
	int total_tasks;
	std::list<task> tasks;
};

static auto header_tpl = "<html>"
	"<head>"
	"<title>" + render::get(&header_context::title) + "</title>"
	"</head>"
	"<body>";

static auto footer_tpl = 
	"<a href=\"/\">Index</a><br/>"
	"<a href=\"/add_task/\">Add new task</a><br/>"
	"<a href=\"/tasks/\">List tasks</a><br/>"
	"</body>"
	"</html>";

/*
 * Index template
 */
static auto index_tpl = header_tpl +
	"<p>Total tasks: <span id=\"total_tasks\">" + render::get(&index_context::total_tasks) + ".</span></p>" +
	footer_tpl;

/*
 * Task added template
 */
static auto task_added_tpl = header_tpl +
	"<p>Description: <span class=\"font-weight: bold;\">" + render::get(&task::get_description) + "</span></p>" +
	footer_tpl;

int
main(int argc, char * argv[])
{
	try
	{
		// Database connection.
		std::cout << "Connecting to database at: " << database_url << std::endl;
		database db(database_url);
		// Creating table
		db.session().create_table<task>();
		// Setting up web application server.
		web::application app(argc, argv);
		
		// Show main page with information about tasks count.
		app.get("/", [&db](web::request &, web::response & res) {
			// Setup scope.
			render::scope s;
			// Header context
			header_context hdr;
			hdr.title = "Web application written in C++.";
			s.push(hdr);
			// Setup template context
			index_context ctx;
			ctx.total_tasks = db.session().query<task>().count();
			s.push(ctx);
			// Render template
			res.stream() << index_tpl(s);
		});

		// Add dummy task to database (its not restful.. yet)
		app.get("/add_task/", [&db](web::request &, web::response & res) {
			session db_session(db.session());

			// Setup scope;
			render::scope s;
			header_context hdr;
			hdr.title = "Added new task.";
			s.push(hdr);
			// Begin transaction
			db_session.begin();
			task t;
			t.id = db.session().query<task>().count() + 1;
			t.description = "New task.";
			db_session.add(t);
			// Add task to scope.
			s.push(t);
			// Render
			res.stream() << task_added_tpl(s);
			// Commit transaction
			db_session.commit();
		});

		// Add dummy task to database (its not restful.. yet)
		app.get("/tasks/", [&db](web::request &, web::response & res) {
			// Get tasks list
			session s(db.session());
			collection<task> results = s.query<task>().all();
			std::list<task> tasks;
			while (collection<task>::const_iterator result = results.next())
			{
				task const & p = *result;
				tasks.push_back(p);
			}
			// Render
			render::scope scope;
			scope.push(tasks);
			header_context hdr;
			hdr.title = "Tasks list.";
			scope.push(hdr);
			// 
			auto tasks_tpl = header_tpl +
				"<ul>" + render::each(tasks, 
					"<li>" + render::get(&task::description) + "</li>") +
				"</ul>" +
				footer_tpl;
			res.stream() << tasks_tpl(scope);
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