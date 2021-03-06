// Copyright [2020] <Copyright Kevin, kevin.lau.gd@gmail.com>

#include "trading_server/risk_management/common/no_self_trade.h"

#include "trading_server/datastruct/contract_table.h"
#include "utils/misc.h"

namespace ft {

bool NoSelfTradeRule::Init(RiskRuleParams* params) {
  order_map_ = params->order_map;
  return true;
}

int NoSelfTradeRule::CheckOrderRequest(const Order* order) {
  if (order->req.direction != Direction::BUY && order->req.direction != Direction::SELL)
    return NO_ERROR;

  auto req = &order->req;
  auto contract = req->contract;

  uint64_t opp_d = OppositeDirection(req->direction);  // 对手方
  const OrderRequest* pending_order;
  for (auto& [oms_order_id, o] : *order_map_) {
    UNUSED(oms_order_id);
    pending_order = &o.req;
    if (pending_order->direction != opp_d) continue;

    // 存在市价单直接拒绝
    if (pending_order->price < 1e-5 || pending_order->type == OrderType::MARKET ||
        (req->direction == Direction::BUY && req->price > pending_order->price - 1e-5) ||
        (req->direction == Direction::SELL && req->price < pending_order->price + 1e-5)) {
      spdlog::error(
          "[RiskMgr] Self trade! Ticker: {}. This Order: "
          "[Direction: {}, Type: {}, Price: {:.2f}]. "
          "Pending Order: [Direction: {}, Type: {}, Price: {:.2f}]",
          contract->ticker, DirectionToStr(req->direction), OrderTypeToStr(req->type), req->price,
          DirectionToStr(pending_order->direction), OrderTypeToStr(pending_order->type),
          pending_order->price);
      return ERR_SELF_TRADE;
    }
  }

  return NO_ERROR;
}

}  // namespace ft
