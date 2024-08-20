#include <graphics.h>
#include <string>
#include<vector>

int idx_current_anim = 0;//用于存储当前动画帧索引

const int PLAYER_ANIM_NUM = 6;// 动画帧总数量常量

const int PLAYER_WIDTH = 80;	//玩家宽度
const int PLAYER_HEIGHT = 80;	//玩家高度
const int SHADOW_WIDTH = 32;	//阴影宽度

const int WINDOW_WIDTH = 1280;
const int WINDOW_HIGHT = 720;

const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75;

int PLAYER_SPEED = 7;//定义角色移动速度

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];

POINT player_pos = { 500,500 };//存储玩家所在位置

#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"MSIMG32.LIB")

bool running = true;
bool is_game_started = false;

inline void putimage_alpha(int x, int y, IMAGE* img)//实现透明通道混叠
{
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

class Atlas//表示动画所使用的图集
{
public:
	Atlas(LPCTSTR path, int num)
	{
		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)
		{
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);
		}
	}

	~Atlas()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
			delete frame_list[i];
	}

public:
	std::vector<IMAGE*> frame_list;
};

Atlas* atlas_player_left;	//玩家向左动画
Atlas* atlas_player_right;	//玩家向右动画
Atlas* atlas_enemy_left;	//敌人向左动画
Atlas* atlas_enemy_right;	//敌人向右动画

void LoadAnimation()		//图片命名比较规律可以使用循环加载图片
{
	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++)
	{
		std::wstring path = L"img/player_left_" + std::to_wstring(i) + L".png";	//使用wstring转换i，并拼凑形成文件路径
		loadimage(&img_player_left[i], path.c_str());							//将图片加入数组中
	}

	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++)
	{
		std::wstring path = L"img/player_right_" + std::to_wstring(i) + L".png";	//使用wstring转换i，并拼凑形成文件路径
		loadimage(&img_player_right[i], path.c_str());								//将图片加入数组中
	}
}

class Animation
{
public:
	Animation(Atlas* atlas, int interval)//路径·数量·帧间隔
	{
		anim_atlas = atlas;
		interval_ms = interval;
	}

	~Animation() = default;

	void play(int x, int y, int delta)	//动画播放函数
	{
		timer += delta;
		if (timer >= interval_ms)
		{
			idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
			timer = 0;
		}

		putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
	}

private:
	int timer = 0;		//动画计时器
	int idx_frame = 0;  //动画帧索引
	int interval_ms = 0;

private:
	Atlas* anim_atlas;
};

//Animation anim_left_player(atlas_player_left, 45);
//Animation anim_right_player(atlas_player_right, 45);

IMAGE img_shadow;

//void DrawPlayer(int delta, int dir_x)//绘制玩家动画
//{
//	int pos_shadow_x = player_pos.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);//计算阴影位置，使其居中
//	int pos_shadow_y = player_pos.y+PLAYER_HEIGHT-8;
//	putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);
//
//	static bool facing_left = false;//判断是否向左
//	if (dir_x < 0)
//		facing_left = true;			//向左
//	else if (dir_x > 0)
//		facing_left = false;		//向右
//
//	if (facing_left)
//		anim_left_player.play(player_pos.x, player_pos.y, delta);
//	else
//		anim_right_player.play(player_pos.x, player_pos.y, delta);
//
//}

class Player
{
public:
	const int FRAME_WIDTH = 80;		//玩家宽度
	const int FRAME_HEIGHT = 80;	//玩家高度

public:
	Player()
	{
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation(atlas_player_left, 45);
		anim_right = new Animation(atlas_player_right, 45);
	}

	~Player()
	{
		delete anim_left;
		delete anim_right;
	}

	void ProcessEvent(const ExMessage& msg)//处理玩家操作信息
	{
		switch (msg.message)
		{
		case WM_KEYDOWN://WM_KEYDOWN表示按键放下
		{
			switch (msg.vkcode)
			{
			case VK_UP:
				is_move_up = true;
				break;
			case VK_DOWN:
				is_move_down = true;
				break;
			case VK_LEFT:
				is_move_left = true;
				break;
			case VK_RIGHT:
				is_move_right = true;
				break;
			}
			break;
		}
		case WM_KEYUP://WM_KEYUP表示按键抬起
		{
			switch (msg.vkcode)
			{
			case VK_UP:
				is_move_up = false;
				break;
			case VK_DOWN:
				is_move_down = false;
				break;
			case VK_LEFT:
				is_move_left = false;
				break;
			case VK_RIGHT:
				is_move_right = false;
				break;
			}
			break;
		}
		}
	}

	void Move()//处理玩家移动
	{
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			player_pos.x += (int)(PLAYER_SPEED * normalized_x);
			player_pos.y += (int)(PLAYER_SPEED * normalized_y);
		}

		//对玩家位置进行校准，使之在窗口内活动
		if (player_pos.x < 0)player_pos.x = 0;
		if (player_pos.y < 0)player_pos.y = 0;
		if (player_pos.x + PLAYER_WIDTH > WINDOW_WIDTH)player_pos.x = WINDOW_WIDTH - PLAYER_WIDTH;
		if (player_pos.y + PLAYER_WIDTH > WINDOW_HIGHT)player_pos.y = WINDOW_HIGHT - PLAYER_WIDTH;
	}

	void Draw(int delta)//绘制玩家
	{
		int pos_shadow_x = player_pos.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);//计算阴影位置，使其居中
		int pos_shadow_y = player_pos.y + PLAYER_HEIGHT - 8;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		static bool facing_left = false;//判断是否向左
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0)
			facing_left = true;			//向左
		else if (dir_x > 0)
			facing_left = false;		//向右

		if (facing_left)
			this->anim_left->play(player_pos.x, player_pos.y, delta);
		else
			this->anim_right->play(player_pos.x, player_pos.y, delta);
	}

	const POINT& GetPosition()const
	{
		return player_pos;
	}

private:
	const int SPEED = 3;
	const int PLAYER_WIDTH = 80;	//玩家宽度
	const int PLAYER_HEIGHT = 80;	//玩家高度
	const int SHADOW_WIDTH = 32;	//阴影宽度

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 500,500 };
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;

};

class Bullet
{
public:
	POINT position = { 0,0 };
public:
	Bullet() = default;
	~Bullet() = default;

	void Draw() const
	{
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(position.x, position.y, RADIUS);
	}

private:
	const int RADIUS = 10;
};

class Enemy
{

public:
	Enemy()
	{
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));
		anim_left = new Animation(atlas_enemy_left, 45);
		anim_right = new Animation(atlas_enemy_right, 45);

		//生成敌人边界
		enum class SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right
		};

		//将敌人放置在地图外边界随机位置
		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge)
		{
		case SpawnEdge::Up:
			position.x = rand() % WINDOW_WIDTH;
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % WINDOW_WIDTH;
			position.y = WINDOW_HIGHT;
			break;
		case SpawnEdge::Left:
			position.x = -FRAME_WIDTH;
			position.y = rand() % WINDOW_HIGHT;
			break;
		case SpawnEdge::Right:
			position.x = WINDOW_WIDTH;
			position.y = rand() % WINDOW_HIGHT;
			break;
		default:
			break;
		}
	}

	bool CheckBulletCollision(const Bullet& bullet)
	{
		//子弹等效为点，判断点是否在敌人矩形内
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HEIGHT;
		return is_overlap_x && is_overlap_y;

	}

	bool CheckPlayerCollision(const Player& player)
	{
		POINT check_position = { position.x + FRAME_WIDTH / 2, position.y + FRAME_HEIGHT / 2 };													//求出敌人中心点坐标
		bool is_overlap_x = check_position.x >= player.GetPosition().x && check_position.x <= player.GetPosition().x + player.FRAME_WIDTH;		//判断x轴碰撞
		bool is_overlap_y = check_position.y >= player.GetPosition().y && check_position.y <= player.GetPosition().y + player.FRAME_HEIGHT;		//判断y轴碰撞

		return is_overlap_x && is_overlap_y;
	}

	void Move(const Player& player)
	{
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(SPEED * normalized_x);
			position.y += (int)(SPEED * normalized_y);
		}

		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;
	}

	void Draw(int delta)
	{
		int pos_shadow_x = position.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + FRAME_HEIGHT - 35;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		if (facing_left)
			anim_left->play(position.x, position.y, delta);
		else
			anim_right->play(position.x, position.y, delta);
	}

	~Enemy()
	{
		delete anim_left;
		delete anim_right;
	}

	void Hurt()			//敌人受击更新敌人存活状态
	{
		alive = false;
	}

	bool CheckAlive()	//获取当前敌人存活状态
	{
		return alive;
	}

private:
	const int SPEED = 2;
	const int FRAME_WIDTH = 80;		//敌人宽度
	const int FRAME_HEIGHT = 80;	//敌人高度
	const int SHADOW_WIDTH = 48;	//阴影宽度

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };
	bool  facing_left = false;
	bool alive = true;				//标志敌人存活状态
};

void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
{
	const int INTERVAL = 100;						//计数间隔
	static int counter = 0;
	if ((++counter) % INTERVAL == 0)				//达到时间间隔生成新的敌人
		enemy_list.push_back(new Enemy());

}

void UpdateBullets(std::vector<Bullet>& bullet_list, const Player& player)//更新子弹，实现子弹实时跟随玩家
{
	const double RADIAL_SPEED = 0.0045;								//径向波动速度
	const double TANGENT_SPEED = 0.0035;							//切向波动速度
	double radian_interval = 2 * 3.14159 / bullet_list.size();		//子弹之间的弧度间隔
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); i++)
	{
		double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;	//当前子弹所在弧度值
		bullet_list[i].position.x = player_position.x + player.FRAME_WIDTH / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.FRAME_HEIGHT / 2 + (int)(radius * cos(radian));
	}
}

void DrawPlayerScore(int score)//绘制玩家得分
{
	static TCHAR text[64];
	_stprintf_s(text, _T("当前玩家得分：%d"), score);

	setbkmode(TRANSPARENT);
	settextcolor(RGB(255, 85, 185));
	outtextxy(10, 10, text);
}

class Button
{
public:
	Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
	{
		region = rect;

		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);
	}

	~Button() = default;

	void Draw()
	{
		switch (status)
		{
		case Status::Idle:
			putimage(region.left, region.top, &img_idle);
			break;
		case Status::Hovered:
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage(region.left, region.top, &img_pushed);
			break;
		}
	}

	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
				status = Status::Hovered;
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
				status = Status::Idle;
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
				status = Status::Pushed;
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed)
				OnClick();
			break;
		default:
			break;
		}
	}

protected:
	virtual void OnClick() = 0;
private:
	enum class Status
	{
		Idle = 0,
		Hovered,
		Pushed
	};

private:
	RECT region;
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;

private:
	//检测鼠标点击
	bool CheckCursorHit(int x, int y)
	{
		return x >= region.left && x <= region.right && y > region.top && y < region.bottom;
	}
};
//游戏开始按钮
class StartGameButton :public Button
{
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~StartGameButton() = default;

protected:
	void OnClick()
	{
		is_game_started = true;

		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);				//播放背景音乐
	}
};
//游戏退出按钮
class QuitGameButton :public Button
{
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~QuitGameButton() = default;

protected:
	void OnClick()
	{
		running = false;
	}
};
int main()
{
	initgraph(1280, 720);

	atlas_player_left = new Atlas(_T("img/player_left_%d.png"), 6);
	atlas_player_right = new Atlas(_T("img/player_right_%d.png"), 6);
	atlas_enemy_left = new Atlas(_T("img/enemy_left_%d.png"), 6);
	atlas_enemy_right = new Atlas(_T("img/enemy_right_%d.png"), 6);

	//添加音乐
	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);			//添加背景音乐
	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL);			//添加子弹击中音效
	mciSendString(_T("open mus/lose.mp3 alias lose"), NULL, 0, NULL);		//添加失败音乐


	int score = 0;
	Player player;
	ExMessage msg;
	IMAGE img_menu;
	IMAGE img_background;
	std::vector<Enemy*>enemy_list;
	std::vector<Bullet>bullet_list(3);

	RECT region_btn_start_game, region_btn_quit_game;

	region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

	region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
	region_btn_quit_game.top = 550;
	region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

	StartGameButton btn_start_game = StartGameButton(region_btn_start_game, _T("img/ui_start_idle.png"), _T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
	QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game, _T("img/ui_quit_idle.png"), _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));

	loadimage(&img_menu, _T("img/menu.png"));
	loadimage(&img_background, _T("img/background.png"));

	BeginBatchDraw();

	while (running)//游戏主循环
	{
		DWORD start_time = GetTickCount();

		while (peekmessage(&msg))
		{
			if (is_game_started)
				player.ProcessEvent(msg);
			else
			{
				btn_start_game.ProcessEvent(msg);
				btn_quit_game.ProcessEvent(msg);
			}
		}
		if (is_game_started)
		{
			//数据处理
			player.Move();
			UpdateBullets(bullet_list, player);
			TryGenerateEnemy(enemy_list);
			for (Enemy* enemy : enemy_list)
				enemy->Move(player);

			//检测敌人和玩家的碰撞
			for (Enemy* enemy : enemy_list)
			{
				if (enemy->CheckPlayerCollision(player))
				{
					static TCHAR text[128];
					mciSendString(_T("stop bgm"), 0, 0, 0);										//停止背景音乐
					mciSendString(_T("play lose repeat from 0"), NULL, 0, NULL);				//播放失败音乐
					_stprintf_s(text, _T("最终得分：%d!"), score);
					MessageBox(GetHWnd(), text, _T("游戏结束"), MB_OK);
					running = false;
					break;
				}
			}
			//检测子弹和敌人碰撞
			for (Enemy* enemy : enemy_list)
			{
				for (const Bullet& bullet : bullet_list)
				{
					if (enemy->CheckBulletCollision(bullet))
					{
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);//播放敌人受击音效
						enemy->Hurt();
						score++;
					}
				}
			}
			//移除生命值归零的敌人
			for (size_t i = 0; i < enemy_list.size(); i++)
			{
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlive())
				{
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;
				}
			}
		}



		//进行渲染
		cleardevice();

		if (is_game_started)
		{
			putimage_alpha(0, 0, &img_background);
			player.Draw(1000 / 144);
			for (Enemy* enemy : enemy_list)
				enemy->Draw(1000 / 144);
			for (const Bullet& bullet : bullet_list)
				bullet.Draw();
			DrawPlayerScore(score);
		}
		else
		{
			putimage(0, 0, &img_menu);
			btn_start_game.Draw();
			btn_quit_game.Draw();
		}

		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delet_time = end_time - start_time;
		if (delet_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delet_time);
		}
	}

	delete atlas_player_left;
	delete atlas_player_right;
	delete atlas_enemy_left;
	delete atlas_enemy_right;

	EndBatchDraw();

	return 0;
}