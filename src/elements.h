#pragma once

constexpr LPCWSTR c_strPaused = L"Paused";
constexpr LPCWSTR c_strScore = L"Score: ";
constexpr LPCWSTR c_strWin = L"Congratulations - You won the game!";
constexpr LPCWSTR c_strFail = L"You failed the game!";
constexpr LPCWSTR c_strControls = L"Space - Pause, Enter - New game, Esc - Quit";
constexpr LPCWSTR c_strLives = L"Lives left:";

struct IDrawable
{
    virtual void Draw(Graphics* graphics, const RectF* rect) = 0;
};

struct IHasBounds
{
    virtual const RectF* GetBounds() const = 0;
};

struct IColorable
{
    virtual void SetColor(const Color& color) = 0;
};

class VisualElement
    : public IDrawable
    , public IHasBounds
    , public IColorable
{
public:
    const RectF* GetBounds() const noexcept override final
    {
        return &rect_;
    }
    void SetColor(const Color& color) noexcept override final
    {
        color_ = color;
    }

    RectF rect_;
    Color color_;
};

//-------------------------------------------------------------------------------------------------------------------------------
class Playgroud
    : public VisualElement
{
public:
    Playgroud(const Color& color, float infoBoardHeight)
        : infoBoardHeight_(infoBoardHeight)
    {
        SetColor(color);
    }

    void Draw(Graphics* graphics, const RectF* rect) override final
    {
        rect_ = *rect;
        rect_.Height -= infoBoardHeight_;

        const SolidBrush sb(color_);
        graphics->FillRectangle(&sb, rect_);
    }
private:
    float infoBoardHeight_{};
};

//-------------------------------------------------------------------------------------------------------------------------------
class GameInformation
    : public VisualElement
{
public:
    GameInformation(const Color& color, float height, size_t livesStart)
        : height_(height)
        , lives_(livesStart)
    {
        SetColor(color);
    }

    void Draw(Graphics* graphics, const RectF* rect) override final
    {
        rect_ = *rect;
        rect_.Y += (rect_.Height - height_);

        const SolidBrush sb(color_);
        graphics->FillRectangle(&sb, rect_);

        DrawPause(graphics);
        DrawScore(graphics);
        DrawVictory(graphics);
        DrawFail(graphics);
        DrawControls(graphics);
        DrawLives(graphics);
    }

    void SetPaused(bool paused) noexcept
    {
        paused_ = paused;
    }

    bool IsPaused() const noexcept
    {
        return paused_;
    }

    void SetVictory() noexcept
    {
        state_ = eState::victory;
    }

    void SetFail() noexcept
    {
        state_ = eState::fail;
    }

    bool IsVictory() const noexcept
    {
        return state_ == eState::victory;
    }

    bool IsFail() const noexcept
    {
        return state_ == eState::fail;
    }

    bool IsOver() const noexcept
    {
        return IsVictory() || IsFail();
    }

    void AddToScore(size_t score) noexcept
    {
        score_ += score;
    }

    bool IsHitTop() const noexcept
    {
        return hittop_;
    }

    void SetHitTop() noexcept
    {
        hittop_ = true;
    }

    void IncrementHits() noexcept
    {
        ++hits_;
    }

    size_t GetHits() const noexcept
    {
        return hits_;
    }

    void SetHitLine(size_t line) noexcept
    {
        linesHits_.insert(line);
    }

    bool IsHitLine(size_t line) const
    {
        return 0 != linesHits_.count(line);
    }

    bool NoMoreLives() const noexcept
    {
        return 0 == lives_;
    }

    void RemoveLife() noexcept
    {
        if (!NoMoreLives())
            --lives_;
    }

private:
    enum class eState
    {
        undefined,
        victory,
        fail,
    };

    void DrawPause(Graphics* graphics)
    {
        if (!IsPaused())
            return;

        const StringFormat sf;
        const SolidBrush brush(Color::Yellow);
        const Gdiplus::Font font(L"Arial", 14.f, FontStyleRegular, UnitPixel);

        const PointF pt(rect_.X + 5, rect_.Y + 5);
        RectF strRect;
        graphics->MeasureString(c_strPaused, -1, &font, pt, &strRect);

        graphics->DrawString(c_strPaused, -1, &font, strRect, &sf, &brush);
    }

    void DrawScore(Graphics* graphics)
    {
        const StringFormat sf;
        const SolidBrush brush(Color::Yellow);
        const Gdiplus::Font font(L"Arial", 14.f, FontStyleRegular, UnitPixel);

        const std::wstring str = c_strScore + std::to_wstring(score_);

        const PointF pt(rect_.X + rect_.Width / 3.f, rect_.Y + 5.f);
        RectF strRect;
        graphics->MeasureString(str.c_str(), -1, &font, pt, &strRect);

        graphics->DrawString(str.c_str(), -1, &font, strRect, &sf, &brush);
    }

    void DrawVictory(Graphics* graphics)
    {
        if (!IsVictory())
            return;

        DrawResult(graphics, c_strWin, Color::Green);
    }

    void DrawFail(Graphics* graphics)
    {
        if (!IsFail())
            return;

        DrawResult(graphics, c_strFail, Color::Red);
    }

    void DrawResult(Graphics* graphics, LPCWSTR result, const Color& color)
    {
        const StringFormat sf;
        const SolidBrush brush(color);
        const Gdiplus::Font font(L"Arial", 14.f, FontStyleRegular, UnitPixel);

        const PointF pt(rect_.X + 5.f, rect_.Y + 25.f);
        RectF strRect;
        graphics->MeasureString(result, -1, &font, pt, &strRect);

        graphics->DrawString(result, -1, &font, strRect, &sf, &brush);
    }

    void DrawControls(Graphics* graphics)
    {
        const StringFormat sf;
        const SolidBrush brush(Color::White);
        const Gdiplus::Font font(L"Arial", 12.f, FontStyleRegular, UnitPixel);

        const PointF pt(rect_.X + 5, rect_.Y + 45.f);
        RectF strRect;
        graphics->MeasureString(c_strControls, -1, &font, pt, &strRect);

        graphics->DrawString(c_strControls, -1, &font, strRect, &sf, &brush);
    }

    void DrawLives(Graphics* graphics)
    {
        const StringFormat sf;
        const SolidBrush brush(Color::Yellow);
        const Gdiplus::Font font(L"Arial", 14.f, FontStyleRegular, UnitPixel);

        const PointF pt(rect_.X + rect_.Width / 1.5f, rect_.Y + 5.f);
        RectF strRect;
        graphics->MeasureString(c_strLives, -1, &font, pt, &strRect);

        graphics->DrawString(c_strLives, -1, &font, strRect, &sf, &brush);

        if (lives_ > 0)
        {
            const SolidBrush sb(Color::Red);
            RectF rect;
            rect.X = strRect.X + strRect.Width + 5;
            rect.Y = strRect.Y;
            rect.Width = 14.f;
            rect.Height = 14.f;

            for (size_t i = 0; i < lives_; ++i)
            {
                graphics->FillEllipse(&sb, rect);
                rect.X += 20;
            }
        }
    }

private:

    size_t lives_{};
    bool    paused_ = true;
    eState  state_ = eState::undefined;
    size_t  score_ = 0;
    bool    hittop_ = false;
    size_t  hits_ = 0;
    using   TLinesHits = std::set<size_t>;
    TLinesHits linesHits_;
    float   height_{};
};

//-------------------------------------------------------------------------------------------------------------------------------
class Player
    : public VisualElement
{
public:
    Player(const Color& color, float height, size_t position, size_t positionsCount)
        : height_(height)
        , position_(position)
        , positionsCount_(positionsCount)
    {
        SetColor(color);
    }

    void Draw(Graphics* graphics, const RectF* rect) override final
    {
        const SizeF size(rect->Width / REAL(positionsCount_), height_);

        rect_.X = rect->GetLeft() + position_ * size.Width;
        rect_.Y = rect->GetBottom() - size.Height;
        rect_.Width = size.Width;
        rect_.Height = size.Height;

        const SolidBrush sb(color_);
        graphics->FillRectangle(&sb, rect_);
    }

    void SplitBy(size_t split) noexcept
    {
        positionsCount_ *= split;
        position_ *= split;
    }

    void MoveLeft() noexcept
    {
        if (position_ > 0)
            --position_;
    }

    void MoveRight() noexcept
    {
        if (position_ < positionsCount_ - 1)
            ++position_;
    }

private:
    size_t positionsCount_{};
    size_t position_{};
    float height_{};
};

//-------------------------------------------------------------------------------------------------------------------------------
class Ball
    : public VisualElement
{
public:
    Ball(const Color& color, float radius, float speed, const PointF& direction, const PointF& start, float maxMovementStep)
        : radius_(radius)
        , speed_(speed)
        , direction_(direction)
        , position_(start)
        , maxStep_(maxMovementStep)
    {
        SetColor(color);
    }

    void Draw(Graphics* graphics, const RectF* rect) override final
    {
        const PointF realPoint(rect->GetLeft() + rect->Width * position_.X, rect->GetTop() + rect->Height * position_.Y);

        rect_.X = realPoint.X - radius_;
        rect_.Y = realPoint.Y - radius_;
        rect_.Width = 2.f * radius_;
        rect_.Height = 2.f * radius_;

        const SolidBrush sb(color_);
        graphics->FillEllipse(&sb, rect_);

        parentRect_ = *rect;
    }

    void CalcNextPosition()
    {
        position_ = CalcNextPos(speed_);
    }

    void SpeedUp(float mul) noexcept
    {
        speed_ *= mul;
    }

    enum class eHitType
    {
        hitInside,
        hitOutside,
    };

    bool HitWithTarget(const IHasBounds* hasBounds)
    {
        // hit priorities depends on movement direction
        eHitType ht = eHitType::hitOutside;

        if (MovingLeft())
        {
            if (MovingUp())
                return HitWithBottom(hasBounds, ht) || HitWithTop(hasBounds, ht);
            else if (MovingDown())
                return HitWithTop(hasBounds, ht) || HitWithBottom(hasBounds, ht);
            else
                return HitWithRight(hasBounds, ht) || HitWithLeft(hasBounds, ht);
        }
        else if (MovingRight())
        {
            if (MovingUp())
                return HitWithBottom(hasBounds, ht) || HitWithTop(hasBounds, ht);
            else if (MovingDown())
                return HitWithTop(hasBounds, ht) || HitWithBottom(hasBounds, ht);
            else
                return HitWithLeft(hasBounds, ht) || HitWithRight(hasBounds, ht);
        }
        else
        {
            if (MovingUp())
                return HitWithBottom(hasBounds, ht) || HitWithTop(hasBounds, ht);
            else if (MovingDown())
                return HitWithTop(hasBounds, ht) || HitWithBottom(hasBounds, ht);
        }

        return false;
    }

    bool HitWithLeft(const IHasBounds* hasBounds, eHitType ht)
    {
        const auto canHit = MovingLeft() || (eHitType::hitOutside == ht && MovingRight());
        if (!canHit)
            return false;

        auto other = hasBounds->GetBounds();

        const bool res = rect_.IntersectsWith(RectF(other->GetLeft(), other->GetTop(), 0, other->Height));
        if (res)
            InverseHorizontalMovement();

        return res;
    }

    bool HitWithRight(const IHasBounds* hasBounds, eHitType ht)
    {
        const auto canHit = MovingRight() || (eHitType::hitOutside == ht && MovingLeft());
        if (!canHit)
            return false;

        auto other = hasBounds->GetBounds();

        const auto res = rect_.IntersectsWith(RectF(other->GetRight(), other->GetTop(), 0, other->Height));
        if (res)
            InverseHorizontalMovement();

        return res;
    }

    bool HitWithTop(const IHasBounds* hasBounds, eHitType ht)
    {
        const auto canHit = MovingUp() || (eHitType::hitOutside == ht && MovingDown());
        if (!canHit)
            return false;

        auto other = hasBounds->GetBounds();

        const auto res = rect_.IntersectsWith(RectF(other->GetLeft(), other->GetTop(), other->Width, 0));
        if (res)
            InverseVerticalMovement();

        return res;
    }

    bool HitWithBottom(const IHasBounds* hasBounds, eHitType ht)
    {
        const auto canHit = MovingDown() || (eHitType::hitOutside == ht && MovingUp());
        if (!canHit)
            return false;

        auto other = hasBounds->GetBounds();

        const auto res = rect_.IntersectsWith(RectF(other->GetLeft(), other->GetBottom(), other->Width, 0));
        if (res)
            InverseVerticalMovement();

        return res;
    }

    bool MovingLeft() const noexcept
    {
        return direction_.X < 0.f;
    }

    bool MovingUp() const noexcept
    {
        return direction_.Y < 0.f;
    }

    bool MovingRight() const noexcept
    {
        return direction_.X > 0.f;
    }

    bool MovingDown() const noexcept
    {
        return direction_.Y > 0.f;
    }

private:

    PointF CalcNextPos(float speed)
    {
        PointF res = position_;
        const auto angle = atanf(fabs(direction_.X) / fabs(direction_.Y));
        auto dx = speed * sinf(angle);
        auto dy = speed * cosf(angle);

        if (std::signbit(direction_.X))
            dx = -dx;
        if (std::signbit(direction_.Y))
            dy = -dy;

        res.X += dx;
        res.Y += dy;

        const bool withinArea = (res.X >= 0.f && res.X <= 1.f && res.Y >= 0.f && res.Y <= 1.f);

        if (!withinArea)
            return CalcNextPos(speed / 2.f);

        // adjust speed to max step allowed
        const auto shift = sqrtf(pow(fabs(dx) * parentRect_.Width, 2.f) + pow(fabs(dy) * parentRect_.Height, 2.f));
        if (shift > maxStep_)
        {
            speed_ *= (maxStep_ / shift);
            return CalcNextPos(speed_);
        }

        return res;
    }

    void InverseHorizontalMovement() noexcept
    {
        direction_.X = -direction_.X;
        direction_.X += GetRandomVectorAddittion();
    }

    void InverseVerticalMovement() noexcept
    {
        direction_.Y = -direction_.Y;
        direction_.Y += GetRandomVectorAddittion();
    }

    static float GetRandomVectorAddittion() noexcept
    {
        // to have some randomity in ricochet logic
        auto val = std::rand();
        val = val % 201; // 0 to 200
        val -= 100; // -100 to 100
        const float add = val / 10000.f; // -0.01 to 0.01
        return add;
    }

    float speed_{};
    PointF position_;
    PointF direction_;
    float radius_{};
    float maxStep_{};
    RectF parentRect_;
};

//-------------------------------------------------------------------------------------------------------------------------------
class Target
    : public VisualElement
{
public:
    Target(size_t line, size_t pos, size_t cost, const Color& color)
        : line_(line)
        , pos_(pos)
        , cost_(cost)
    {
        SetColor(color);
    }

    void Draw(Graphics* graphics, const RectF* rect) override final
    {
        rect_ = *rect;
        const SolidBrush sb(color_);
        graphics->FillRectangle(&sb, rect_);
    }

    size_t GetCost() const noexcept
    {
        return cost_;
    }

    size_t GetLine() const noexcept
    {
        return line_;
    }

    size_t GetPos() const noexcept
    {
        return pos_;
    }

private:
    size_t line_{};
    size_t pos_{};
    size_t cost_{};
};

//-------------------------------------------------------------------------------------------------------------------------------
class Targets
    : public IDrawable
{
public:
    using TLines = std::map<size_t, std::pair<Color, size_t>>;

    Targets(const TLines& lines, size_t targetsInLine, float margin, float topMargin, float targetHeight)
        : margin_(margin)
        , topMargin_(topMargin)
        , lineSize_(targetsInLine)
        , targetHeight_(targetHeight)
    {
        linesBase_ = lines.size();
        for (const auto& line : lines)
        {
            targets_.emplace_back(std::vector< Target >());

            auto& newLine = targets_.back();

            for (size_t j = 0; j < targetsInLine; ++j)
            {
                newLine.emplace_back(Target(line.first, j, line.second.second, line.second.first));
            }
        }
    }

    void Draw(Graphics* graphics, const RectF* rect) override final
    {
        for (size_t i = 0; i < targets_.size(); ++i)
        {
            auto& line = targets_[i];

            for (size_t j = 0; j < line.size(); ++j)
            {
                auto& target = line[j];

                RectF targetRect;
                targetRect.Width = (rect->Width - margin_ * (lineSize_ + 1)) / lineSize_;
                targetRect.Height = targetHeight_;
                targetRect.X = rect->GetLeft() + (margin_ * (target.GetPos() + 1)) + targetRect.Width * target.GetPos();
                targetRect.Y = rect->GetTop() + topMargin_ + (targetRect.Height + margin_) * (linesBase_ - target.GetLine() - 1);

                target.Draw(graphics, &targetRect);
            }
        }
    }

    const Target* GetTargetHitWithBall(Ball* ball)
    {
        if (ball->MovingDown())
        {
            // moving down - start search target from top lines
            for (auto it = targets_.rbegin(); it != targets_.rend(); ++it)
            {
                for (auto& target : *it)
                {
                    if (ball->HitWithTarget(&target))
                    {
                        return &target;
                    }
                }
            }
        }
        else
        {
            // not moving down - start search target from bottom lines
            for (auto& line : targets_)
            {
                for (auto& target : line)
                {
                    if (ball->HitWithTarget(&target))
                    {
                        return &target;
                    }
                }
            }
        }
        return nullptr;
    }

    bool IsEmpty() const noexcept
    {
        return targets_.empty();
    }

    void RemoveTarget(const Target* target)
    {
        for (auto it1 = targets_.begin(); it1 != targets_.end(); ++it1)
        {
            auto it = std::find_if(it1->begin(), it1->end(), [target](const auto& elem) { return &elem == target; });
            if (it1->end() != it)
            {
                it1->erase(it);

                if (it1->empty())
                {
                    targets_.erase(it1);
                }
                break;
            }
        }
    }

private:
    std::vector<std::vector< Target > > targets_;
    float margin_{};
    float topMargin_{};
    float targetHeight_{};
    size_t lineSize_{};
    size_t linesBase_{};
};
