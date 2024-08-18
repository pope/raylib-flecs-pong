use ffi::{CheckCollisionCircleRec, IsKeyDown, IsKeyPressed, Rectangle};
use flecs::{
	pipeline::{OnUpdate, OnValidate, PreUpdate},
	rest::Rest,
	Disabled,
};
use flecs_ecs::prelude::*;
use raylib::prelude::*;
use std::ffi::c_void;

const WINDOW_WIDTH: i32 = 1280;
const WINDOW_HEIGHT: i32 = 720;

const PADDLE_WIDTH: i32 = 30;
const PADDLE_HEIGHT: i32 = 160;
const PADDLE_GAP: i32 = 10;

const BALL_RADIUS: f32 = 20.;

#[derive(Debug, Component)]
struct Position {
	x: f32,
	y: f32,
}

impl From<&Position> for raylib::ffi::Vector2 {
	fn from(pos: &Position) -> Self {
		Self { x: pos.x, y: pos.y }
	}
}

#[derive(Debug, Component)]
struct Velocity {
	x: f32,
	y: f32,
}

#[derive(Debug, Component)]
struct Ball;

#[derive(Debug, Component)]
struct Paddle;

#[derive(Debug, Component)]
struct Player {
	up_key: KeyboardKey,
	down_key: KeyboardKey,
}

#[derive(Debug, Component)]
struct Cpu;

fn clamp<T>(input: T, min: T, max: T) -> T
where
	T: PartialOrd,
{
	let d = if min > input { min } else { input };
	if max < d {
		max
	} else {
		d
	}
}

fn main() {
	let (mut rl, thread) = raylib::init()
		.size(WINDOW_WIDTH, WINDOW_HEIGHT)
		.title("Pope Pong")
		.msaa_4x()
		.log_level(TraceLogLevel::LOG_DEBUG)
		.build();

	let world = World::new();
	world.set_target_fps(60.);
	world.set::<Rest>(Rest::default());
	world.import::<flecs_ecs::addons::stats::Stats>();

	// Systems
	let mut ball_query = world.query::<&Position>().with::<&Ball>().build();
	let mut collisions_query =
		world.query::<&Position>().with::<&Paddle>().build();
	{
		world
			.system_named::<()>("CheckPause")
			.kind::<PreUpdate>()
			.run(|it| {
				if unsafe { IsKeyPressed(KeyboardKey::KEY_P as i32) } {
					let sys = EntityView::new_from(it.world(), OnUpdate::ID);
					if sys.has_id(Disabled::ID) {
						sys.remove_id(Disabled::ID);
					} else {
						sys.add_id(Disabled::ID);
					}
				}
			});
		world
			.system_named::<(&mut Position, &mut Velocity)>("MoveBall")
			.with::<&Ball>()
			.kind::<OnUpdate>()
			.each(|(p, v)| {
				p.x += v.x;
				p.y += v.y;

				if (p.x + BALL_RADIUS >= WINDOW_WIDTH as f32)
					| (p.x - BALL_RADIUS <= 0.)
				{
					v.x *= -1.;
				}
				if (p.y + BALL_RADIUS >= WINDOW_HEIGHT as f32)
					| (p.y - BALL_RADIUS <= 0.)
				{
					v.y *= -1.;
				}
			});
		world
			.system_named::<(&mut Position, &Velocity, &Player)>("MovePlayer")
			.kind::<OnUpdate>()
			.each(|(p, v, player)| {
				let mut dir = 0.;
				if unsafe { IsKeyDown(player.up_key as i32) } {
					dir -= 1.;
				}
				if unsafe { IsKeyDown(player.down_key as i32) } {
					dir += 1.;
				}

				p.y += v.y * dir;
				p.y = clamp(
					p.y,
					PADDLE_GAP as f32,
					(WINDOW_HEIGHT - PADDLE_HEIGHT - PADDLE_GAP) as f32,
				);
			});
		world
			.system_named::<(&mut Position, &Velocity)>("MoveCpu")
			.with::<&Cpu>()
			.set_context(
				&mut ball_query as *mut Query<&Position> as *mut c_void,
			)
			.kind::<OnUpdate>()
			.each_iter(|mut it, _index, (cpu_p, cpu_v)| {
				let ball_query = unsafe { it.context::<Query<&Position>>() };
				ball_query.each(|ball_p| {
					let dir = if cpu_p.y + PADDLE_HEIGHT as f32 / 2. > ball_p.y
					{
						-1.
					} else {
						1.
					};
					cpu_p.y += cpu_v.y * dir;
				});
				cpu_p.y = clamp(
					cpu_p.y,
					PADDLE_GAP as f32,
					(WINDOW_HEIGHT - PADDLE_HEIGHT - PADDLE_GAP) as f32,
				);
			});
		world
			.system_named::<(&Position, &mut Velocity)>("CheckCollisions")
			.with::<&Ball>()
			.set_context(
				&mut collisions_query as *mut Query<&Position> as *mut c_void,
			)
			.kind::<OnValidate>()
			.each_iter(|mut it, _index, (ball_p, ball_v)| {
				let collisions_query =
					unsafe { it.context::<Query<&Position>>() };
				collisions_query.each(|paddle_p| {
					let rec = Rectangle {
						x: paddle_p.x,
						y: paddle_p.y,
						width: PADDLE_WIDTH as f32,
						height: PADDLE_HEIGHT as f32,
					};
					if unsafe {
						CheckCollisionCircleRec(ball_p.into(), BALL_RADIUS, rec)
					} {
						ball_v.x *= -1.;
					}
				});
			});
	}

	// Entities
	{
		world
			.entity_named("objs.Ball")
			.set(Position {
				x: WINDOW_WIDTH as f32 / 2.,
				y: WINDOW_HEIGHT as f32 / 2.,
			})
			.set(Velocity { x: 7., y: 7. })
			.add::<Ball>();

		world
			.entity_named("objs.PlayerPaddle")
			.set(Position {
				x: PADDLE_GAP as f32,
				y: WINDOW_HEIGHT as f32 / 2. - PADDLE_HEIGHT as f32 / 2.,
			})
			.set(Velocity { x: 0., y: 5. })
			.set(Player {
				up_key: KeyboardKey::KEY_W,
				down_key: KeyboardKey::KEY_S,
			})
			.add::<Paddle>();

		world
			.entity_named("objs.CpuPaddle")
			.set(Position {
				x: (WINDOW_WIDTH - PADDLE_WIDTH - PADDLE_GAP) as f32,
				y: WINDOW_HEIGHT as f32 / 2. - PADDLE_HEIGHT as f32 / 2.,
			})
			.set(Velocity { x: 0., y: 5. })
			.add::<Paddle>()
			.add::<Cpu>();
	}

	// Game Loop
	while !rl.window_should_close() {
		world.progress();

		let mut d = rl.begin_drawing(&thread);
		d.clear_background(Color::BLACK);

		// Background
		{
			let mid_point_x = WINDOW_WIDTH / 2;
			let grid_y: i32 = WINDOW_HEIGHT / 4;

			d.draw_line(mid_point_x, 0, mid_point_x, grid_y, Color::WHITE);
			d.draw_line(
				mid_point_x,
				grid_y * 3,
				mid_point_x,
				WINDOW_HEIGHT,
				Color::WHITE,
			);
			d.draw_circle_lines(
				mid_point_x,
				grid_y * 2,
				grid_y as f32,
				Color::WHITE,
			);
		}

		// Ball
		world.new_query::<(&Position, &Ball)>().each(|(p, _ball)| {
			d.draw_circle_v(p, BALL_RADIUS, Color::WHITE);
		});

		// Paddles
		world
			.new_query::<(&Position, &Paddle)>()
			.each(|(p, _paddle)| {
				d.draw_rectangle(
					p.x as i32,
					p.y as i32,
					PADDLE_WIDTH,
					PADDLE_HEIGHT,
					Color::WHITE,
				);
			});

		d.draw_fps(WINDOW_WIDTH - 100, 20);
	}
}
